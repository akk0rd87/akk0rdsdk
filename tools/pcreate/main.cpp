#include <cstdio>
#include <iostream>


void PrintHelp()
{
    puts("  Params:");
    puts("");
    puts("  -p  <project_name>");
    puts("  -d  <project_destination_folder>");
    puts("  -b  <bundle>");
    puts("  -ab <android_bundle>           {optional. If not specified then bundle will be taken}");
    puts("  -ib <ios_bundle>               {optional. If not specified then bundle will be taken}");
    puts("");
    puts("  Example:"); 
    puts("");
    puts("  pcreate -p JCross -d C:\\Files\\JCRoss -b org.popapp.jcross -ib org.popapp.jcrossfree");
}

bool CheckParams(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
        puts(argv[i]);

    return true;
}

std::string Replace(const char* strBase, const char *s1, const char *s2)
{
    std::string str(strBase);
    std::string findstr(s1);
    std::string replacestr(s2);
    std::string::size_type index;
    while ((index = str.find(findstr)) != std::string::npos)
        str.replace(index, findstr.size(), replacestr);

    return str;
}

void main(int argc, char* argv[])
{
    printf("\n%s\n", Replace("it is my project with {$PROJECT_NAME}", "{$PROJECT_NAME}", "JCross").c_str());

    return;
    PrintHelp();
    if (CheckParams(argc, argv))
    {

    }
}