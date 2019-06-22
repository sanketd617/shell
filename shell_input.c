
int getInput(char * ip){
    char prompt[CWD_LEN];
    getPrompt(prompt);
    char * buff = readline(prompt);
    if(strlen(buff) != 0){
        add_history(buff);
        strcpy(ip, buff);
        return 1;
    }
    return 0;
}
