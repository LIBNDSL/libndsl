/**
 * @file V8Test.cc
 * @brief
 * 实体的测试
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/

#include <v8.h>
#include <string>
using namespace v8;
using namespace std;
int main(int argc, char *argv[])
{
    char jscode[] =
        "function cook(){console.log('开始做饭。');var p = new \
        promise(function(resolve, reject) \
        {setTimeout(function() \
        {console.log('做饭完毕！');resolve('鸡蛋炒饭。');},1000);});return p;}\
        function eat(){consle.log('开始吃饭。');\
        var p = new promise(function(resolve, reject) {setTimeout(function() \
        {console.log('吃饭完毕！');resolve('一堆碗筷。');\
        },2000);});return p;}\
        cook().then(eat).then(function(data) { console.log(data); });";
    // 创建一个局部Handle监视器，当它析构时自动清理所有Local类型的对象
    HandleScope handle_scope;
    // 创建一个永久上下文范围变量，它在构造函数中进入上下文，析构函数中退出上下文
    Persistent<Context> context = Context::New();
    // 脚本的编译和运行必须处于某个上下文中
    Context::Scope context_scope(context);

    // 创建并初始化一个字符串对象，这个串将作为脚本代码，实现两个字符串的相加
    printf("%s\n", jscode);
    Handle<String> source = v8::String::New(jscode);
    // 编译脚本代码串，产生一个本地脚本对象
    myConsole = new console.Console(out, err);
    myconsole.log('helloworld');
    printf("开始编译！\n");
    Handle<Script> script = Script::Compile(source);
    // 运行脚本对象，获得一个返回值
    printf("开始run！\n");
    Handle<Value> result = script->Run();
    // 释放永久上下文对象，将被垃圾回收机制自动回收
    context.Dispose();
    // 把返回值转换成本地字符串后输出至屏幕，打印结果是 Hello, World!
    String::AsciiValue ascii(result);
    printf("%s\n", *ascii);
    return 0;
}

/*
#include "v8.h"

using namespace ndsl;
using namespace net;
using namespace v8;

int main(int argc, char *argv[])
{
    // 初始化V8
    V8::InitializeICU();
    V8::InitializeExternalStartupData(argv[0]);
    Platform *platform = platform::CreateDefaultPlatform();
//Platform用来管理isolate，确定他是在后台线程还是前台线程运行，管理线程池等。
    V8::InitializePlatform(platform);

    V8::Initialize();
    // 创建一个新的Isolate
    ArrayBufferAllocator allocator; // 设置数组分配器
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = &allocator;

    // Isolate::New()生成一个实例
    Isolate *isolate = Isolate::New(create_params);
表示一个单独的v8引擎实例，Isolate有完全独立的状态，对象在isolate之间不能共享。
    {
        Isolate::Scope isolate_scope(isolate);
        // 创建一个分配在栈上的handle scope.
        HandleScope handle_scope(isolate); //管理本地Handle

        // 创建一个context.
        Local<Context> context = Context::New(isolate);
        // 关联context
        Context::Scope context_scope(context);

        // 创建一个包含JavaScript代码的字符串
        Local<String> source =
            String::NewFromUtf8(
                isolate, "'Hello' + ', World!'", NewStringType::kNormal)
                .ToLocalChecked();
        // 编译JavaScript代码
        Local<Script> script =
            Script::Compile(context, source).ToLocalChecked();
        // 运行代码并产生结果
        Local<Value> result = script->Run(context).ToLocalChecked();

        String::Utf8Value utf8(result);
        printf("%s\n", *utf8);
    }
    // 释放资源
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete platform;
    return 0;
}
*/