#include <omniGlass/constants.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <omniGlass/omniglass.h>
#include <mpv/client.h>

double plug_omniglass_last_slide_checked = 0.0;
int plug_omniglass_is_seeking = 0;

mpv_handle *plug_omniglass_mpv_ctx;

void plug_omniglass_on_bottom_edge_slide(double value){
    //deadzone (only run command past a certain value.)
    if(value == 0.0)
        return;
    printf("edge slide detected.\n");
    plug_omniglass_last_slide_checked += (value * 0.001);
    if((plug_omniglass_last_slide_checked) < 0.08 && (plug_omniglass_last_slide_checked > -0.08)){
         return;
    }
    if(!(plug_omniglass_is_seeking)){
        char *command = "seek";
        char arg[36];
        // char **textset = malloc(sizeof(char *) * 2);
        sprintf(arg, "%f", plug_omniglass_last_slide_checked);
        const char *textset[3] = {command, arg, NULL};
        printf("%s %s\n", textset[0], textset[1]);
        // mpv_command(plug_omniglass_mpv_ctx, textset);
        mpv_command_async(plug_omniglass_mpv_ctx, 201, textset);
        const char *cmd_show_progress[2] = {"show_progress", NULL};
        mpv_command_async(plug_omniglass_mpv_ctx, 200, cmd_show_progress);
        // mpv_get_property(plug_omniglass_mpv_ctx, "time_pos)
        plug_omniglass_last_slide_checked = 0.0;
        plug_omniglass_is_seeking = 1;
    }

}
int mpv_open_cplugin(mpv_handle *handle){
    plug_omniglass_mpv_ctx = handle;
    mpv_wait_event(handle,-1);
    struct omniglass *omniglass;
    if (omniglass_init(&omniglass) != OMNIGLASS_RESULT_SUCCESS){
        fprintf(stderr, "could not start omniglass instance\n");
        return 0;
    }
    printf("mpv-omniglass step1: omniglass initialized.\n");
    
    omniglass_listen_gesture_edge(omniglass, plug_omniglass_on_bottom_edge_slide, OMNIGLASS_EDGE_BOTTOM);
    
    while (1){
        mpv_event *event = mpv_wait_event(handle, 0.004);
        int exit = 0;
        omniglass_step(omniglass);
        // printf("stepping through touchpad state machine\n");
        
        switch(event->event_id){
            case MPV_EVENT_SHUTDOWN:
                return 0;
                break;
            // case MPV_EVENT_SEEK:
            //     plug_omniglass_is_seeking = 0;
            //     break;
            case MPV_EVENT_COMMAND_REPLY:
                if(event->reply_userdata == 200)
                    plug_omniglass_is_seeking = 0;
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
