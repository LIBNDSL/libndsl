CommandHandler::CommandHandler()
{
    userState.currentWorkingDir = "/mnt/ftpdir";
    userState.mode = "PASV";
    userState.username = "ranxiangjun";
    userState.password = "123456";
    userState.isLogin = false;
}

CommandHandler::~CommandHandler(){}

int CommandHandler::writeState(int fd, string& content)
{
    
}

int commandHandler::parseCommand(string &content)
{
    // get the paramater's hash value
    typedef std::uint64_t hash_t;
     
    constexpr hash_t prime = 0x100000001B3ull;
    constexpr hash_t basis = 0xCBF29CE484222325ull;
     
    hash_t hash_(char const* str)
    {
        hash_t ret{basis};     
        while(*str){
            ret ^= *str;
            ret *= prime;
            str++;
            }    
        return ret;
    }

    constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
    {
        return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;
    }

    constexpr unsigned long long operator "" _hash(char const* p, size_t)
    {
        return hash_compile_time(p);
    }

    // parse the paramater
    switch hash_(getFirstParam(content).c_str())
    {
        // check the username
        case "USER"_hash:
            if (getSecendParam(content) == "ranxiangjun")
            {
                writeState(commFd, (string)"331 username is ok, need password\r\n");
                break;
            }
            else
            {
                writeState(commFd, (string)"501 invalid username\r\n");
                break;
            }
            
        // check the user's password
        case "PASS"_hash:
            if (getSecendParam(content) == "123456")
            {
                userState.isLogin = ture;
                writeState(commFd, (string)"230 login successfully\r\n");
                break;
            }
            writeState(commFd, (string)"501 incorrect password\r\n");
            break;

        // change working directory
        case "CWD"_hash:
            if (!userState.isLogin)
            {
                writeState(commFd, (string)"332 need user account\r\n");
                break;
            }
            if (opendir(getSecendParam(content)) == NULL)
            {
                writeState(commFd, (string)"501 invalid directory\r\n");
                break;
            }
            userState.currentWorkingDir = getSecendParam(content);
            writeState(commFd, (string)"200, change directory successfully\r\n");
            break;

        // sever listens and change to passive mode
        case "PASV"_hash:
            if (!userState.isLogin)
            {
                writeState(commFd, (string)"332 need user account\r\n");
                break;
            }
            userState.mode = "PASV";
            writeState(commFd, (string)"200 mode is PASV now\r\n");
            break;

        // tell server port and change to active mode 
        case "PORT"_hash:
            if (!userState.isLogin)
            {
                writeState(commFd, (string)"332 need user account\r\n");
                break;
            }
            userState.mode = "PORT";
            writeState(commFd, (string)"200 mode is PORT now\r\n");
            break;

        // download file from server
        case "RETR"_hash:

        // upload file to server
        case "STOR"_hash:

        // return the named list,or return working dir list
        case "LIST"_hash:

        // disconnect to server
        case "QUIT"_hash:
            userState.isLogin == false;
            writeState(commFd, (string)"200 be unlogin now\r\n");
            break;
}

string commandHandler::getCurrentWorkingDir()
{
    return userState.currentWorkingDir;
}

string commandHandler::getMode()
{
    return userState.mode;
}

string commandHandler::getFirstParam(string &content)
{
    return content.substring(0, content.find("\0"));
}

string commandHandler::getSecendParam(string &content)
{
    return content.substr(content.find("\0") + 1);
}
      









