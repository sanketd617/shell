#include <readline/history.h>

void displayHistory(){
    HISTORY_STATE *hist = history_get_history_state();
    HIST_ENTRY **list = history_list();
    for (int i = 0; i < hist->length; i++) {
        printf ("%3d\t%s\n", i, list[i]->line);
    }
}
