#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

void execute_command(std::vector<char*> cmd)
{
    std::cout << "フォーク前: " << getpid() << std::endl;
    pid_t pid = fork();
    if(pid < 0){ // fork失敗
        std::cerr << "[Error] fork(2) failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    if(pid == 0){ // 子プロセス
        std::cout << "子プロセス: (fork-pid, getpid) = (" << pid << ", " << getpid() << ")" << std::endl;
        execvp(cmd[0], cmd.data());
    }
    if(pid > 0){ // 親プロセス
        int status;
        std::cout << "親プロセス: (fork-pid, getpid) = (" << pid << ", " << getpid() << ")" << std::endl;
        std::cout << "waitpid(" << pid << ")" << std::endl;
        pid_t ret = waitpid(pid, &status, 0); 
        std::cout << "waitの返り値: " << ret << std::endl;
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