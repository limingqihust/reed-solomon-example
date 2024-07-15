#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <chrono>
std::vector<std::vector<int>> distribution;

void ReadDistribution();
std::string GetNodeName(int i);
double EncodeFileJ(int j);
void Clear();
double SendFileJ(int j, const std::string& node_name);
double SendEncodedFileJ(int j, const std::string& node_name);


/**
 * compute encoding time
 */
int main() {
    ReadDistribution();


    double total_time = 0;
    for (int i = 0;i < distribution.size(); i++) {
        const std::string node_name = GetNodeName(i+2);
        for (auto j: distribution[i]) {
            // scp file[j] to node_name
            total_time += SendEncodedFileJ(j, node_name);
        }
    }
    std::cout << "total_time: " << total_time << std::endl;



    // double total_time = 0;
    // for (int i = 0; i<=99; i++) {
    //     total_time += EncodeFileJ(i);
    //     Clear();
    // }
    // std::cout << "total_time: " << total_time << std::endl;
}

void ReadDistribution() {
    std::ifstream infile("/root/MergeCDC/Distribution/Distribution");
    std::vector<std::vector<int>> data;
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(infile, line)) {
        lines.push_back(line);
    }

    if (!lines.empty()) {
        lines.pop_back();
    }

    for (const auto& ln : lines) {
        std::istringstream iss(ln);
        std::vector<int> row;
        int number;
        while (iss >> number) {
            row.push_back(number);
        }
        data.push_back(row);
    }

    for (const auto& row : data) {
        for (const auto& num : row) {
            std::cout << num << " ";
        }
        std::cout << std::endl;
    }
    distribution = data;
}

std::string GetNodeName(int i) {
    std::string node_name;
    if (1 <= i && i <= 9) {
        node_name = "node0" + std::to_string(i);
    } else {
        node_name = "node" + std::to_string(i);
    }
    return node_name;
}

double EncodeFileJ(int j) {
    std::cout << "encode " << j << " start" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    pid_t pid = fork();
    assert(pid != -1);
    std::string filename = "/root/MergeCDC/Input/tera10G_" + std::to_string(j);
    if (pid == 0) {             // child
        char *args[] = {(char*)"./encoder", (char*)filename.c_str(), (char*)"20", (char*)"2", (char*)"reed_sol_van", (char*)"8", (char*)"64", (char*)"1024", NULL};
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {                    // parent
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "encode " << j << " done" << std::endl;
        } else {
            std::cout << "encode " << j << " error" << std::endl; 
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "elapsed: " << elapsed.count() << std::endl;
    return elapsed.count();
}

void Clear() {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 子进程
        char *args[] = {(char*)"rm", (char*)"-rf", (char*)"./Coding", NULL};
        execvp(args[0], args);

        // 如果 execvp 返回，说明它失败了
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // 父进程
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            // waitpid 失败
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            // 子进程正常退出
            int exitStatus = WEXITSTATUS(status);
            printf("Child program exited with status %d\n", exitStatus);
        } else if (WIFSIGNALED(status)) {
            // 子进程被信号终止
            int signal = WTERMSIG(status);
            printf("Child program terminated by signal %d\n", signal);
        } else {
            printf("Child program did not terminate normally\n");
        }
    }
}


double SendFileJ(int j, const std::string& node_name) {
    std::cout << "send " << j << " to " << node_name << " start" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    pid_t pid = fork();
    assert(pid != -1);
    std::string filename = "/root/MergeCDC/Input/tera10G_" + std::to_string(j);
    std::string dst = "root@" + node_name + ":/home/";
    // std::string dst = "root@" + node_name + ":/home/tera10G_" + std::to_string(j) + "_temp";
    if (pid == 0) {             // child
        char * args[] = {(char*)"scp", (char*)filename.c_str(), (char*)dst.c_str(), NULL};
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {                    // parent
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "send " << j << " to " << node_name << " done" << std::endl;
        } else {
            std::cout << "send " << j << " to " << node_name << " error" << std::endl; 
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "elapsed: " << elapsed.count() << std::endl;
    return elapsed.count();
}


/**
 * encode file j and send to node_name
 */
double SendEncodedFileJ(int j, const std::string& node_name) {
    EncodeFileJ(j);

    std::cout << "send encoded " << j << " to " << node_name << " start" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    pid_t pid = fork();
    assert(pid != -1);
    std::string filename = "/root/Jerasure/Examples/Coding";
    std::string dst = "root@" + node_name + ":/home/";
    if (pid == 0) {             // child
        char * args[] = {(char*)"scp", (char*)"-r", (char*)filename.c_str(), (char*)dst.c_str(), NULL};
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {                    // parent
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "send encoded " << j << " to " << node_name << " done" << std::endl;
        } else {
            std::cout << "send encoded " << j << " to " << node_name << " error" << std::endl; 
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "send encoded file time: " << elapsed.count() << std::endl;


    Clear();
    return elapsed.count();
}


