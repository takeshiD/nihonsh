#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <memory>

class Command {
public:
    int argc;
    std::vector<char*> argv;
    Command* next;
    Command(): argc(0), argv(), next(nullptr) {}
    Command(int _argc, std::vector<char*> _argv, Command* _next): argc(argc), argv(_argv), next(next) {}
};

void execute_command(char** cmd)
{
    pid_t pid = fork();
    if(pid < 0){
        std::cerr << "[Error] fork(2) failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    if(pid == 0){
        execve(cmd[0], cmd, environ);
    }
    if(pid > 0){
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
    return 0;
}