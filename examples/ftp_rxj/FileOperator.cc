FileOperator::FileOperator(){}
FileOperator::~FileOperator(){}
int FileOperator::deleteFile(string path)
{
    if (remove(path.c_str()) < 0)
    {
        cout<< "delete failed"<<endl;
        return -1;
    }
}

int FileOperator::creatFile(string path)
{
    ofstring outfile(path.c_str(), ofstring::out|ofstring::app|ofstring::binary);
    if(!outfile)
    {
        cout <<"create file failed"<<endl;
        return -1;
    }
    outfile.close();
}

int FileOperator::storeFile(string& path, char* content)
{
    ofstring outfile(path.c_str(), ofstring::out|ofstring::app|ofstring::binary);
    if(!outfile)
    {
        cout<<"open file failed"<<endl;
        return -1;
    }
    outfile << content;
    outfile.close();
    return 1;
}

int FileOperator::getFile(string &filename)
{

