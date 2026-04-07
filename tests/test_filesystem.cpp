#include <gtest/gtest.h>
#include <zen/filesystem.h>

using namespace zen;
using namespace zen::fs;

TEST(PathTest, Construction) {
    path p1("/usr/local/bin");
    EXPECT_TRUE(p1.is_absolute());
    EXPECT_EQ(p1.filename(), "bin");
    
    path p2("relative/path");
    EXPECT_TRUE(p2.is_relative());
    EXPECT_EQ(p2.filename(), "path");
}

TEST(PathTest, Operators) {
    path p1("/usr");
    path p2("local/bin");
    path p3 = p1 / p2;
    
    EXPECT_EQ(p3.string(), "/usr/local/bin");
}

TEST(PathTest, ParentAndStem) {
    path p("/home/user/document.txt");
    
    EXPECT_EQ(p.parent_path().string(), "/home/user");
    EXPECT_EQ(p.stem(), "document");
    EXPECT_EQ(p.extension(), ".txt");
}

TEST(PathTest, Normalization) {
    path p1("/usr/local/../bin");
    EXPECT_EQ(p1.string(), "/usr/bin");
    
    path p2("/usr/./local/bin");
    EXPECT_EQ(p2.string(), "/usr/local/bin");
}

TEST(DirectoryTest, CreateAndRemove) {
    path test_dir = "/tmp/libzen_test_dir";
    
    EXPECT_TRUE(create_directory(test_dir));
    EXPECT_TRUE(exists(test_dir));
    EXPECT_TRUE(remove_directory(test_dir));
    EXPECT_FALSE(exists(test_dir));
}

TEST(DirectoryTest, CreateDirectories) {
    path test_dir = "/tmp/libzen_test/nested/directory";
    
    EXPECT_TRUE(create_directories(test_dir));
    EXPECT_TRUE(exists(test_dir));
    EXPECT_TRUE(remove_all(test_dir.parent_path().parent_path().parent_path()));
}

TEST(DirectoryIteratorTest, BasicIteration) {
    path test_dir = "/tmp/libzen_iter_test";
    create_directories(test_dir);
    
    std::ofstream(test_dir / "file1.txt").close();
    std::ofstream(test_dir / "file2.txt").close();
    
    int count = 0;
    directory_iterator end;
    for (directory_iterator it(test_dir); it != end; ++it) {
        count++;
    }
    
    EXPECT_EQ(count, 2);
    
    remove_all(test_dir);
}

TEST(DirectoryIteratorTest, RecursiveIteration) {
    path test_dir = "/tmp/libzen_recursive_test";
    create_directories(test_dir);
    create_directories(test_dir / "subdir");
    
    std::ofstream(test_dir / "file1.txt").close();
    std::ofstream(test_dir / "subdir" / "file2.txt").close();
    
    int count = 0;
    recursive_directory_iterator end;
    for (recursive_directory_iterator it(test_dir); it != end; ++it) {
        if (it->is_file()) {
            count++;
        }
    }
    
    EXPECT_EQ(count, 2);
    
    remove_all(test_dir);
}
