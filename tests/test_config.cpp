#include <gtest/gtest.h>
#include <zen/config.h>

using namespace zen;
using namespace zen::config;

TEST(IniConfigTest, LoadAndSave) {
    ini_config config;
    
    config.set("server", "host", "127.0.0.1");
    config.set("server", "port", "8080");
    config.set("server", "debug", "true");
    
    config.set("database", "host", "localhost");
    config.set("database", "name", "testdb");
    config.set("database", "user", "admin");
    
    EXPECT_TRUE(config.save("/tmp/test_config.ini"));
    
    ini_config loaded;
    EXPECT_TRUE(loaded.load("/tmp/test_config.ini"));
    
    EXPECT_EQ(loaded.get("server", "host").as_string(), "127.0.0.1");
    EXPECT_EQ(loaded.get("server", "port").as_string(), "8080");
    EXPECT_EQ(loaded.get("server", "debug").as_bool(), true);
    EXPECT_EQ(loaded.get("database", "name").as_string(), "testdb");
}

TEST(IniConfigTest, SectionsAndKeys) {
    ini_config config;
    
    config.set("section1", "key1", "value1");
    config.set("section1", "key2", "value2");
    config.set("section2", "key1", "value3");
    
    EXPECT_TRUE(config.has_section("section1"));
    EXPECT_TRUE(config.has_section("section2"));
    EXPECT_FALSE(config.has_section("section3"));
    
    EXPECT_TRUE(config.has_key("section1", "key1"));
    EXPECT_TRUE(config.has_key("section1", "key2"));
    EXPECT_FALSE(config.has_key("section1", "key3"));
    
    auto sections = config.sections();
    EXPECT_EQ(sections.size(), 2);
    
    auto keys = config.keys("section1");
    EXPECT_EQ(keys.size(), 2);
}

TEST(IniConfigTest, Remove) {
    ini_config config;
    
    config.set("section1", "key1", "value1");
    config.set("section1", "key2", "value2");
    
    EXPECT_TRUE(config.remove_key("section1", "key1"));
    EXPECT_FALSE(config.has_key("section1", "key1"));
    
    EXPECT_TRUE(config.remove_section("section1"));
    EXPECT_FALSE(config.has_section("section1"));
}
