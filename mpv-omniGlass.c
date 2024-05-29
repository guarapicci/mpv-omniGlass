#include <omniGlass/constants.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <omniGlass/omniglass.h>
#include <mpv/client.h>

#define PLUG_OMNIGLASS_DEFAULT_SLIDE_SENSITIVITY_SEEK 0.09
#define PLUG_OMNIGLASS_DEFAULT_SLIDE_THRESHOLD 0.03

#define PLUG_OMNIGLASS_DEFAULT_SLIDE_SENSITIVITY_VOLUME 7

struct plug_omniglass_state {
    mpv_handle *mpv;
    struct omniglass *omniglass;
    //bottom edge parameters
    double sensitivity_bottom;
    double threshold;
    double last_slide_checked_seek;
    int is_seeking;
    //right edge parameters
    double sensitivity_right;
};

void plug_omniglass_on_bottom_edge_slide(double value, void *data){
    //deadzone (only run command past a certain value.)
    if(value == 0.0)
        return;
    // printf("edge slide detected.\n");
    struct plug_omniglass_state *state = (struct plug_omniglass_state *)data;
    state->last_slide_checked_seek += (value * state->sensitivity_bottom);
    if((state->last_slide_checked_seek < state->threshold) && 
        (state->last_slide_checked_seek > ((-1) * state->threshold))){
         return;
    }
    if(!(state->is_seeking)){
        char *command = "seek";
        char arg[36];
        // char **textset = malloc(sizeof(char *) * 2);
        sprintf(arg, "%f", state->last_slide_checked_seek);
        const char *textset[3] = {command, arg, NULL};
        printf("\nomniglass: edge slide -> %s %s\n", textset[0], textset[1]);
        // mpv_command(plug_omniglass_mpv_ctx, textset);
        mpv_command_async(state->mpv, 201, textset);
        const char *cmd_show_progress[3] = {"osd-bar", "show-progress", NULL};
        mpv_command_async(state->mpv, 200, cmd_show_progress);
        // mpv_get_property(plug_omniglass_mpv_ctx, "time_pos)
        state->last_slide_checked_seek = 0.0;
        state->is_seeking = 1;
    }

}

void plug_omniglass_on_right_edge_slide(double value, void *data){
    struct plug_omniglass_state *state = (struct plug_omniglass_state *)data;
    
    double changed_volume = 0.0;
    mpv_get_property(state->mpv, "ao-volume", MPV_FORMAT_DOUBLE, &changed_volume);
    changed_volume += (value *state->sensitivity_right);
    if(changed_volume<0.0)
        changed_volume=0.0;
    if(changed_volume>100.0)
        changed_volume=100.0;
    mpv_set_property(state->mpv, "ao-volume",MPV_FORMAT_DOUBLE,&changed_volume);
}
int mpv_open_cplugin(mpv_handle *handle){
    struct plug_omniglass_state state;
    state.mpv = handle;
    mpv_wait_event(handle,-1);
    if (omniglass_init(&(state.omniglass)) != OMNIGLASS_RESULT_SUCCESS){
        fprintf(stderr, "could not start omniglass instance\n");
        return 0;
    }
    printf("mpv-omniglass step1: omniglass initialized.\n");
    state.is_seeking=0;
    state.last_slide_checked_seek=0.0;
    state.sensitivity_bottom=PLUG_OMNIGLASS_DEFAULT_SLIDE_SENSITIVITY_SEEK;
    state.threshold=PLUG_OMNIGLASS_DEFAULT_SLIDE_THRESHOLD;
    omniglass_listen_gesture_edge(state.omniglass, plug_omniglass_on_bottom_edge_slide, OMNIGLASS_EDGE_BOTTOM, &state);
    state.sensitivity_right = PLUG_OMNIGLASS_DEFAULT_SLIDE_SENSITIVITY_VOLUME;
    omniglass_listen_gesture_edge(state.omniglass, plug_omniglass_on_right_edge_slide, OMNIGLASS_EDGE_RIGHT, &state);
    
    while (1){
        mpv_event *event = mpv_wait_event(handle, 0.004);
        int exit = 0;
        omniglass_step(state.omniglass);
        // printf("stepping through touchpad state machine\n");
        
        switch(event->event_id){
            case MPV_EVENT_SHUTDOWN:
                return 0;
                break;
            // case MPV_EVENT_SEEK:
            //     plug_omniglass_is_seeking = 0;PLUG_OMNIGLASS_DEFAULT_SLIDE_THRESHOLD
            //     break;PLUG_OMNIGLASS_DEFAULT_SLIDE_THRESHOLD
            case MPV_EVENT_COMMAND_REPLY:
                switch(event->reply_userdata){
                    case 200:
                        state.is_seeking = 0;
                        break;
                }
                break;
        }
        // usleep(4);
    }
    return 0;
}


// // Build with: gcc -o simple.so simple.c `pkg-config --cflags mpv` -shared -fPIC
// // Warning: do not link against libmpv.so! Read:
// //    https://mpv.io/manual/master/#linkage-to-libmpv
// // The pkg-config call is for adding the proper client.h include path.
// 
// #include <stddef.h>
// #include <stdio.h>
// #include <stdlib.h>
// 
// #include <mpv/client.h>
// 
// int mpv_open_cplugin(mpv_handle *handle)
// {
//     printf("Hello world from C plugin '%s'!\n", mpv_client_name(handle));
//     while (1) {
//         mpv_event *event = mpv_wait_event(handle, -1);
//         printf("Got event: %d\n", event->event_id);
//         if (event->event_id == MPV_EVENT_SHUTDOWN)
//             break;
//     }
//     return 0;
// }
