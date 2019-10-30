#ifndef __AKK0RD_TESTFS_MANAGER_H__
#define __AKK0RD_TESTFS_MANAGER_H__

#include <cstring>

extern Statistic statistic;

const static char* Subdir1 = "/Full_length__Directory_111";
const static char* RecursiveSubdir1 = "/Full_length__Directory_recursive_111/RecrsiveSubdir_One";
const static char* RecursiveSubdir2 = "/Full_length__Directory_recursive_222///RecrsiveSubdir_TWO////";
const static char* RecursiveSubdir3 = "/Full_length__Directory_recursive_333///RecrsiveSubdir_THREE//////DirNumber";

class TestFS
{
private:
    std::string Subdir1_File1         ;
    std::string Subdir1_File2         ;
    std::string Subdir1_File3         ;
    std::string Subdir1_File4         ;

    std::string RecursiveSubdir1_File1;
    std::string RecursiveSubdir1_File2;
    std::string RecursiveSubdir1_File3;
    std::string RecursiveSubdir1_File4;

    std::string RecursiveSubdir3_File1;
    std::string RecursiveSubdir3_File2;
    std::string RecursiveSubdir3_File3;
    std::string RecursiveSubdir3_File4;

    void RunCreates();
    void RunDeletes();
    void RunRenames();
    void RunDeleteRecursive();
    void RunCheckLists();
    void RunCheckFilesData();
public:
    void Run()
    {
        RunCreates();
        RunDeleteRecursive();
        RunCreates();
        RunCheckLists();
        RunCheckFilesData();
        RunRenames();
        RunDeleteRecursive();
    };

    TestFS()
    {
        Subdir1_File1          = BWrapper::GetInternalWriteDir() + std::string(Subdir1) + "/File1.txt";
        Subdir1_File2          = BWrapper::GetInternalWriteDir() + std::string(Subdir1) + "/File2.dmg";
        Subdir1_File3          = BWrapper::GetInternalWriteDir() + std::string(Subdir1) + "/File3.java";
        Subdir1_File4          = BWrapper::GetInternalWriteDir() + std::string(Subdir1) + "/File4.dll";

        RecursiveSubdir1_File1 = BWrapper::GetInternalWriteDir() + std::string(RecursiveSubdir1) + "/File1.txt";
        RecursiveSubdir1_File2 = BWrapper::GetInternalWriteDir() + std::string(RecursiveSubdir1) + "/File2.dmg";
        RecursiveSubdir1_File3 = BWrapper::GetInternalWriteDir() + std::string(RecursiveSubdir1) + "/File3.java";
        RecursiveSubdir1_File4 = BWrapper::GetInternalWriteDir() + std::string(RecursiveSubdir1) + "/File4.dll";

        RecursiveSubdir3_File1 = BWrapper::GetInternalWriteDir() + std::string(RecursiveSubdir3) + "/File_abc1.txt";
        RecursiveSubdir3_File2 = BWrapper::GetInternalWriteDir() + std::string(RecursiveSubdir3) + "/File_abc2.dmg";
        RecursiveSubdir3_File3 = BWrapper::GetInternalWriteDir() + std::string(RecursiveSubdir3) + "/File_abc3.java";
        RecursiveSubdir3_File4 = BWrapper::GetInternalWriteDir() + std::string(RecursiveSubdir3) + "/File_abc4.dll";
    }
};

void TestFS::RunCreates()
{
    std::string Fname;
    decltype(BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary)) F;
    char buffer1[] = "Is's a AKKORD_SDK for win32, ios and Android develop.\n";
    char buffer2[] = "Unit test for testing FS API.\n";
    char buffer0[] = "There is the place [0] where 0 code will be situated.\n";
    buffer0[20] = '\0';
    bool checker = true;

    // CREATE DIRS
    // create base write dir
    UT_CHECK(BWrapper::DirCreate(BWrapper::GetInternalWriteDir().c_str()));
    UT_CHECK(BWrapper::DirExists(BWrapper::GetInternalWriteDir().c_str()));

    // create RecursiveSubdir1
    UT_CHECK(BWrapper::DirCreate(std::string(BWrapper::GetInternalWriteDir() + Subdir1).c_str()));
    UT_CHECK(BWrapper::DirExists(std::string(BWrapper::GetInternalWriteDir() + Subdir1).c_str()));
    UT_CHECK(BWrapper::DirCreate(std::string(BWrapper::GetInternalWriteDir() + Subdir1).c_str()));
    UT_CHECK(BWrapper::DirExists(std::string(BWrapper::GetInternalWriteDir() + Subdir1).c_str()));

    // create RecursiveSubdir1
    UT_CHECK(BWrapper::DirCreate(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir1).c_str()));
    UT_CHECK(BWrapper::DirExists(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir1).c_str()));
    UT_CHECK(BWrapper::DirCreate(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir1).c_str()));
    UT_CHECK(BWrapper::DirExists(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir1).c_str()));

    // create RecursiveSubdir2
    UT_CHECK(BWrapper::DirCreate(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir2).c_str()));
    UT_CHECK(BWrapper::DirExists(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir2).c_str()));
    UT_CHECK(BWrapper::DirCreate(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir2).c_str()));
    UT_CHECK(BWrapper::DirExists(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir2).c_str()));

    // create RecursiveSubdir3
    UT_CHECK(BWrapper::DirCreate(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir3).c_str()));
    UT_CHECK(BWrapper::DirExists(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir3).c_str()));
    UT_CHECK(BWrapper::DirCreate(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir3).c_str()));
    UT_CHECK(BWrapper::DirExists(std::string(BWrapper::GetInternalWriteDir() + RecursiveSubdir3).c_str()));

    // CREATE FILES
    Fname = Subdir1_File1;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWrite(F, buffer1, sizeof(buffer1) - 1, 1));
    UT_CHECK(BWrapper::FileWriteFormatted(F, "\n"));
    UT_CHECK(BWrapper::FileWriteFormatted(F, "\n"));
    UT_CHECK(BWrapper::FileWrite(F, buffer2, sizeof(buffer2) - 1, 1));
    UT_CHECK(BWrapper::FileWriteFormatted(F, "\n"));
    UT_CHECK(BWrapper::FileWriteFormatted(F, "\n"));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    Fname = Subdir1_File2;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWrite(F, buffer2, sizeof(buffer2) - 1, 1));
    UT_CHECK(BWrapper::FileWrite(F, buffer1, sizeof(buffer1) - 1, 1));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    Fname = Subdir1_File3;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWrite(F, buffer2, sizeof(buffer2) - 1, 1));
    UT_CHECK(BWrapper::FileWrite(F, buffer0, sizeof(buffer0) - 1, 1));
    UT_CHECK(BWrapper::FileWrite(F, buffer0, 1, sizeof(buffer0) - 1));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    Fname = Subdir1_File4;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    checker = true;
    for (int i = 0; i < 1000; i++)
    {
        checker = checker & (BWrapper::FileWrite(F, buffer2, sizeof(buffer2) - 1, 1));
        checker = checker & (BWrapper::FileWrite(F, buffer0, sizeof(buffer0) - 1, 1));
        checker = checker & (BWrapper::FileWrite(F, buffer2, 1, sizeof(buffer2) - 1));
        checker = checker & (BWrapper::FileWrite(F, buffer0, 1, sizeof(buffer0) - 1));
    }
    UT_CHECK(checker);
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    Fname = RecursiveSubdir1_File1;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s\n", "world"));
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s\n", "peace"));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    Fname = RecursiveSubdir1_File2;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s %d %s\n", "america", 1004123, "True"));
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s %d %s\n", "USA"    , 3131312, "False"));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));

    Fname = RecursiveSubdir1_File3;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s %d %s\n", "america", 1004123, "True"));
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s %d %s\n", "USA", 3131312, "False"));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    Fname = RecursiveSubdir1_File4; // write nothing
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    // write few time with append mode
    Fname = RecursiveSubdir3_File1;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s\n", "world"));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::AppendBinary);
    UT_CHECK(BWrapper::FileWriteFormatted(F, "It's append mode %s\n", "peace"));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    // write few time with append mode
    checker = true;
    Fname = RecursiveSubdir3_File2;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s %d %u\n", "world", 100500, 100501));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::AppendBinary);
    checker = checker & (BWrapper::FileWrite(F, buffer2, sizeof(buffer2) - 1, 1));
    checker = checker & (BWrapper::FileWrite(F, buffer0, sizeof(buffer0) - 1, 1));
    checker = checker & (BWrapper::FileWrite(F, buffer2, 1, sizeof(buffer2) - 1));
    checker = checker & (BWrapper::FileWrite(F, buffer0, 1, sizeof(buffer0) - 1));
    UT_CHECK(checker);
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);

    // write few time with append mode multiple times
    Fname = RecursiveSubdir3_File3;
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::WriteBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s %d %u\n", "world", 100500, 100501));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);
    checker = true;
    for (int i = 0; i < 200; i++)
    {
        F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::AppendBinary);
        checker = checker & (BWrapper::FileWrite(F, buffer2, sizeof(buffer2) - 1, 1));
        checker = checker & (BWrapper::FileWrite(F, buffer0, sizeof(buffer0) - 1, 1));
        checker = checker & (BWrapper::FileWrite(F, buffer2, 1, sizeof(buffer2) - 1));
        checker = checker & (BWrapper::FileWrite(F, buffer0, 1, sizeof(buffer0) - 1));
        BWrapper::FileClose(F);
        checker = checker & (BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
        checker = checker & !F;
    }
    UT_CHECK(checker);

    // open in append mode
    Fname = RecursiveSubdir3_File4;
    if (BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem))
        BWrapper::FileDelete(Fname.c_str());
    UT_CHECK(!BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    F = BWrapper::FileOpen(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem, BWrapper::FileOpenMode::AppendBinary);
    UT_CHECK(F);
    UT_CHECK(BWrapper::FileWriteFormatted(F, "Hello %s %d %u\n", "world", 100500, 100501));
    BWrapper::FileClose(F);
    UT_CHECK(BWrapper::FileExists(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!F);
};

void TestFS::RunDeleteRecursive()
{
    //const static char* Subdir1 = "/Full_length__Directory_111";
    //const static char* RecursiveSubdir1 = "/Full_length__Directory_recursive_111/RecrsiveSubdir_One";
    //const static char* RecursiveSubdir2 = "/Full_length__Directory_recursive_222/\\//RecrsiveSubdir_TWO//\\//\\";
    //const static char* RecursiveSubdir3 = "/Full_length__Directory_recursive_333/\\//RecrsiveSubdir_THREE//\\//\\//DirNumber";

    UT_CHECK(BWrapper::DirExists         (std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_111").c_str()));
    UT_CHECK(BWrapper::DirRemoveRecursive(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_111").c_str()));
    UT_CHECK(!BWrapper::DirExists        (std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_111").c_str()));

    UT_CHECK(BWrapper::DirExists         (std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_111/").c_str()));
    UT_CHECK(BWrapper::DirRemoveRecursive(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_111/").c_str()));
    UT_CHECK(!BWrapper::DirExists        (std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_111/").c_str()));

    UT_CHECK(BWrapper::DirExists         (std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_222/").c_str()));
    UT_CHECK(BWrapper::DirRemoveRecursive(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_222/").c_str()));
    UT_CHECK(!BWrapper::DirExists        (std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_222/").c_str()));

    UT_CHECK(BWrapper::DirExists         (std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_333/").c_str()));
    UT_CHECK(BWrapper::DirRemoveRecursive(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_333/").c_str()));
    UT_CHECK(!BWrapper::DirExists        (std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_333/").c_str()));
}

void TestFS::RunRenames()
{
    // CHECK IF EXISTS
    UT_CHECK(BWrapper::FileExists(Subdir1_File1.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(Subdir1_File2.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(Subdir1_File3.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(Subdir1_File4.c_str(), BWrapper::FileSearchPriority::FileSystem));

    UT_CHECK(BWrapper::FileExists(RecursiveSubdir1_File1.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(RecursiveSubdir1_File2.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(RecursiveSubdir1_File3.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(RecursiveSubdir1_File4.c_str(), BWrapper::FileSearchPriority::FileSystem));

    UT_CHECK(BWrapper::FileExists(RecursiveSubdir3_File1.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(RecursiveSubdir3_File2.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(RecursiveSubdir3_File3.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(RecursiveSubdir3_File4.c_str(), BWrapper::FileSearchPriority::FileSystem));

    // RENAMES
    UT_CHECK(BWrapper::FileRename(Subdir1_File1.c_str(), std::string(Subdir1_File1 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(Subdir1_File2.c_str(), std::string(Subdir1_File2 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(Subdir1_File3.c_str(), std::string(Subdir1_File3 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(Subdir1_File4.c_str(), std::string(Subdir1_File4 + ".tmp").c_str()));

    UT_CHECK(BWrapper::FileRename(RecursiveSubdir1_File1.c_str(), std::string(RecursiveSubdir1_File1 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(RecursiveSubdir1_File2.c_str(), std::string(RecursiveSubdir1_File2 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(RecursiveSubdir1_File3.c_str(), std::string(RecursiveSubdir1_File3 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(RecursiveSubdir1_File4.c_str(), std::string(RecursiveSubdir1_File4 + ".tmp").c_str()));

    UT_CHECK(BWrapper::FileRename(RecursiveSubdir3_File1.c_str(), std::string(RecursiveSubdir3_File1 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(RecursiveSubdir3_File2.c_str(), std::string(RecursiveSubdir3_File2 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(RecursiveSubdir3_File3.c_str(), std::string(RecursiveSubdir3_File3 + ".tmp").c_str()));
    UT_CHECK(BWrapper::FileRename(RecursiveSubdir3_File4.c_str(), std::string(RecursiveSubdir3_File4 + ".tmp").c_str()));

    // CHECK IF NEW-NAME FILES EXISTS
    UT_CHECK(BWrapper::FileExists(std::string(Subdir1_File1 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(Subdir1_File2 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(Subdir1_File3 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(Subdir1_File4 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));

    UT_CHECK(BWrapper::FileExists(std::string(RecursiveSubdir1_File1 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(RecursiveSubdir1_File2 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(RecursiveSubdir1_File3 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(RecursiveSubdir1_File4 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));

    UT_CHECK(BWrapper::FileExists(std::string(RecursiveSubdir3_File1 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(RecursiveSubdir3_File2 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(RecursiveSubdir3_File3 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(BWrapper::FileExists(std::string(RecursiveSubdir3_File4 + ".tmp").c_str(), BWrapper::FileSearchPriority::FileSystem));
};

void TestFS::RunCheckLists()
{
    unsigned all, search;
    DirContentReader   Dr;
    DirContentElement* De;

    // Search items in WriteDirectory root
    all = search = 0;
    UT_CHECK(Dr.Open(BWrapper::GetInternalWriteDir().c_str()));
    while (Dr.Next(De))
    {
        ++all;

        if (De->isDir)
        {
            if (De->Name == "Full_length__Directory_111" || De->Name == "Full_length__Directory_recursive_111" || De->Name == "Full_length__Directory_recursive_222" || De->Name == "Full_length__Directory_recursive_333")
                ++search;
        }
    }
    Dr.Close();
    UT_CHECK(all == search && all == 4);

    // Full_length__Directory_111
    all = search = 0;
    UT_CHECK(Dr.Open(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_111").c_str()));
    while (Dr.Next(De))
    {
        ++all;

        if (De->isDir == 0)
        {
            if (De->Name == "File1.txt" || De->Name == "File2.dmg" || De->Name == "File3.java" || De->Name == "File4.dll")
                ++search;
        }
    }
    Dr.Close();
    UT_CHECK(all == search && all == 4);


    // Full_length__Directory_recursive_111
    all = search = 0;
    UT_CHECK(Dr.Open(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_111").c_str()));
    while (Dr.Next(De))
    {
        ++all;

        if (De->isDir)
        {
            if (De->Name == "RecrsiveSubdir_One")
                ++search;
        }
    }
    Dr.Close();
    UT_CHECK(all == search && all == 1);


    // \Full_length__Directory_recursive_111/RecrsiveSubdir_One
    all = search = 0;
    UT_CHECK(Dr.Open(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_111/RecrsiveSubdir_One").c_str()));
    while (Dr.Next(De))
    {
        ++all;

        if (De->isDir == 0)
        {
            if (De->Name == "File1.txt" || De->Name == "File2.dmg" || De->Name == "File3.java" || De->Name == "File4.dll")
                ++search;
        }
    }
    Dr.Close();
    UT_CHECK(all == search && all == 4);


    // Full_length__Directory_recursive_222
    all = search = 0;
    UT_CHECK(Dr.Open(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_222").c_str()));
    while (Dr.Next(De))
    {
        ++all;

        if (De->isDir)
        {
            if (De->Name == "RecrsiveSubdir_TWO")
                ++search;
        }
    }
    Dr.Close();
    UT_CHECK(all == search && all == 1);

    // Full_length__Directory_recursive_222/RecrsiveSubdir_TWO
    all = search = 0;
    UT_CHECK(Dr.Open(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_222/RecrsiveSubdir_TWO").c_str()));
    while (Dr.Next(De))
    {
        ++all;
    }
    Dr.Close();
    UT_CHECK(all == 0);


    // Full_length__Directory_recursive_333
    all = search = 0;
    UT_CHECK(Dr.Open(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_333").c_str()));
    while (Dr.Next(De))
    {
        ++all;

        if (De->isDir)
        {
            if (De->Name == "RecrsiveSubdir_THREE")
                ++search;
        }
    }
    Dr.Close();
    UT_CHECK(all == search && all == 1);

    // Full_length__Directory_recursive_333/RecrsiveSubdir_THREE
    all = search = 0;
    UT_CHECK(Dr.Open(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_333/RecrsiveSubdir_THREE").c_str()));
    while (Dr.Next(De))
    {
        ++all;

        if (De->isDir)
        {
            if (De->Name == "DirNumber")
                ++search;
        }
    }
    Dr.Close();
    UT_CHECK(all == search && all == 1);


    // Full_length__Directory_recursive_333/RecrsiveSubdir_THREE/DirNumber
    all = search = 0;
    UT_CHECK(Dr.Open(std::string(BWrapper::GetInternalWriteDir() + "/Full_length__Directory_recursive_333/RecrsiveSubdir_THREE/DirNumber").c_str()));
    while (Dr.Next(De))
    {
        ++all;

        if (De->isDir == 0)
        {
            if (De->Name == "File_abc1.txt" || De->Name == "File_abc2.dmg" || De->Name == "File_abc3.java" || De->Name == "File_abc4.dll")
                ++search;
        }
    }
    Dr.Close();
    UT_CHECK(all == search && all == 4);
}

void TestFS::RunCheckFilesData()
{
    FileReader fr;
    std::string Fname, Line;
    unsigned lineptr, readed;
    char buffer[1024];

    //char buffer1[] = "Is's a AKKORD_SDK for win32, ios and Android develop.\n";
    char buffer2[] = "Unit test for testing FS API.\n";
    char buffer0[] = "There is the place [0] where 0 code will be situated.\n";
    buffer0[20] = '\0';

    Fname = Subdir1_File1;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    lineptr = 0;
    while (fr.ReadLine(Line))
    {
        ++lineptr;
        switch (lineptr)
        {
        case 1:
            UT_CHECK(Line == "Is's a AKKORD_SDK for win32, ios and Android develop.");
            break;
        case 2:
        case 3:
        case 5:
        case 6:
            UT_CHECK(Line == "");
            break;
        case 4:
            UT_CHECK(Line == "Unit test for testing FS API.");
            break;
        default:
            UT_CHECK(1 == 0); // error
            break;
        }
    }
    fr.Close();
    UT_CHECK(6 == lineptr);


    Fname = Subdir1_File2;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    lineptr = 0;
    while (fr.ReadLine(Line))
    {
        ++lineptr;
        switch (lineptr)
        {
        case 1:
            UT_CHECK(Line == "Unit test for testing FS API.");
            break;
        case 2:
            UT_CHECK(Line == "Is's a AKKORD_SDK for win32, ios and Android develop.");
            break;
        default:
            UT_CHECK(1 == 0); // error
            break;
        }
    }
    fr.Close();
    UT_CHECK(2 == lineptr);


    Fname = Subdir1_File3;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Unit test for testing FS API.");
    UT_CHECK(fr.Read(buffer, sizeof(buffer0) - 1, readed));
    UT_CHECK(sizeof(buffer0) - 1 == readed);
    UT_CHECK(memcmp(buffer, buffer0, sizeof(buffer0) - 1) == 0);
    UT_CHECK(fr.Read(buffer, sizeof(buffer0) - 1, readed));
    UT_CHECK(sizeof(buffer0) - 1 == readed);
    UT_CHECK(memcmp(buffer, buffer0, sizeof(buffer0) - 1) == 0);
    fr.Close();

    Fname = Subdir1_File4;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    for (int i = 0; i < 1000; i++)
    {
        UT_CHECK(fr.Read(buffer, sizeof(buffer2) - 1, readed));
        UT_CHECK(sizeof(buffer2) - 1 == readed);
        UT_CHECK(memcmp(buffer, buffer2, sizeof(buffer2) - 1) == 0);

        UT_CHECK(fr.Read(buffer, sizeof(buffer0) - 1, readed));
        UT_CHECK(sizeof(buffer0) - 1 == readed);
        UT_CHECK(memcmp(buffer, buffer0, sizeof(buffer0) - 1) == 0);

        UT_CHECK(fr.Read(buffer, sizeof(buffer2) - 1, readed));
        UT_CHECK(sizeof(buffer2) - 1 == readed);
        UT_CHECK(memcmp(buffer, buffer2, sizeof(buffer2) - 1) == 0);

        UT_CHECK(fr.Read(buffer, sizeof(buffer0) - 1, readed));
        UT_CHECK(sizeof(buffer0) - 1 == readed);
        UT_CHECK(memcmp(buffer, buffer0, sizeof(buffer0) - 1) == 0);
    }
    fr.Close();

    Fname = RecursiveSubdir1_File1;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello world");
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello peace");
    fr.Close();

    Fname = RecursiveSubdir1_File2;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello america 1004123 True");
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello USA 3131312 False");
    fr.Close();

    Fname = RecursiveSubdir1_File3;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello america 1004123 True");
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello USA 3131312 False");
    fr.Close();

    Fname = RecursiveSubdir1_File4;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!fr.ReadLine(Line));
    fr.Close();

    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(!fr.Read(buffer, 100, readed));
    fr.Close();

    Fname = RecursiveSubdir3_File1;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello world");
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "It's append mode peace");
    fr.Close();

    Fname = RecursiveSubdir3_File2;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello world 100500 100501");
    UT_CHECK(fr.Read(buffer, sizeof(buffer2) - 1, readed));
    UT_CHECK(sizeof(buffer2) - 1 == readed);
    UT_CHECK(memcmp(buffer, buffer2, sizeof(buffer2) - 1) == 0);
    UT_CHECK(fr.Read(buffer, sizeof(buffer0) - 1, readed));
    UT_CHECK(sizeof(buffer0) - 1 == readed);
    UT_CHECK(memcmp(buffer, buffer0, sizeof(buffer0) - 1) == 0);
    fr.Close();

    Fname = RecursiveSubdir3_File3;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello world 100500 100501");
    for (int i = 0; i < 200; i++)
    {
        UT_CHECK(fr.Read(buffer, sizeof(buffer2) - 1, readed));
        UT_CHECK(sizeof(buffer2) - 1 == readed);
        UT_CHECK(memcmp(buffer, buffer2, sizeof(buffer2) - 1) == 0);

        UT_CHECK(fr.Read(buffer, sizeof(buffer0) - 1, readed));
        UT_CHECK(sizeof(buffer0) - 1 == readed);
        UT_CHECK(memcmp(buffer, buffer0, sizeof(buffer0) - 1) == 0);

        UT_CHECK(fr.Read(buffer, sizeof(buffer2) - 1, readed));
        UT_CHECK(sizeof(buffer2) - 1 == readed);
        UT_CHECK(memcmp(buffer, buffer2, sizeof(buffer2) - 1) == 0);

        UT_CHECK(fr.Read(buffer, sizeof(buffer0) - 1, readed));
        UT_CHECK(sizeof(buffer0) - 1 == readed);
        UT_CHECK(memcmp(buffer, buffer0, sizeof(buffer0) - 1) == 0);
    }
    fr.Close();

    Fname = RecursiveSubdir3_File4;
    UT_CHECK(fr.Open(Fname.c_str(), BWrapper::FileSearchPriority::FileSystem));
    UT_CHECK(fr.ReadLine(Line));
    UT_CHECK(Line == "Hello world 100500 100501");
    fr.Close();
}

#endif // __AKK0RD_TESTFS_MANAGER_H__
