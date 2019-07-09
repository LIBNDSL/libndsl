#include <iostream>
#include <dlfcn.h>
#include <stdio.h>
#include "ndsl/utils/Plugin.h"
#include "ndsl/net/Clientplugin.h"
using namespace std;

typedef Plugin *(*Creat)(int);
typedef void (*ADD)(void *);
// void func(void *para) { std::cout << "hello world!\n " << std::endl; }

int main(int argc, char **argv)
{
    void *p = NULL;
    Client *CLI = NULL;
    p = dlopen(
        "/home/zzt/winuxshare/ndsl/build/bin/libplugin.so",
        RTLD_NOW | RTLD_DEEPBIND);
    if (!p) {
        cout << "cannot open library: " << dlerror() << endl;
        return -1;
    }

    // 返回creatplugin这个函数！！！
    Creat fun = (Creat) dlsym(p, "CreatPlugin");

    if (!fun) {
        cout << "cannot load sysbol:  " << dlerror() << endl;
        dlclose(p);
        return -1;
    }

    void *p2 = NULL;
    p2 = dlopen(
        "/home/zzt/winuxshare/ndsl/build/bin/libclientplugin.so",
        RTLD_NOW | RTLD_DEEPBIND);
    if (!p2) {
        cout << "cannot open library: " << dlerror() << endl;
        return -1;
    }

    ADD fun2 = (ADD) dlsym(p2, "Client::add");
    if (!fun2) {
        cout << "cannot load sysbol:  " << dlerror() << endl;
        dlclose(p);
        return -1;
    }

    CLI = (Client *) fun(1);
    if (!CLI) { printf("plugin == NULL !\n"); }
    printf("before doit\n");
    fun2(NULL);
    // CLI->doit(fun2, NULL);
    printf("after doit\n");
    return 0;
}
