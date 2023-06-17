#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

void execute_command(std::vector<char*> cmd)
{
    pid_t pid = fork();
    if(pid < 0){ // fork失敗
        std::cerr << "[Error] fork(2) failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    if(pid == 0){ // 親プロセス
        execvp(cmd[0], cmd.data());
    }
    if(pid > 0){ // 子プロセス
        int status;
        pid_t ret = waitpid(pid, &status, 0);
        if(ret < 0){
            std::cerr << "[Error] waitpid(2) failed" << std::endl;
            exit(EXIT_FAILURE);
        }
        if(WIFEXITED(status)){
            // nop
        }
        else if(WIFSIGNALED(status)){
            std::cerr << "[Info] stop signaled" << std::endl;
        }
        else{
            std::cerr << "[Error] abnormal exit" << std::endl;
        }
        return;
    }
}

int main(int argc, char** argv)
{
    std::vector<char*> cmd = {"ls", "-la"};
    execute_command(cmd);
    return 0;
}