#include<gtest/gtest.h>

#include <../include/gz4d/URI.h>

TEST(URI,DefaultConstructor)
{
    gz4d::URI u;
    EXPECT_EQ("",std::string(u));
}

TEST(URI,CopyConstructor)
{
    gz4d::URI u("http://foo.bar:123/path?query#frag");
    gz4d::URI u2(u);
    EXPECT_EQ(std::string(u),std::string(u2));
}


TEST(URI,EmptyString)
{
    gz4d::URI u("");
    EXPECT_EQ("",std::string(u));
}

TEST(URI,MissingSchemeException)
{
    EXPECT_THROW(gz4d::URI("missing.scheme/oops"), gz4d::URIException);
}

TEST(URI,SimpleHTTPNoPath)
{
    gz4d::URI u("http://vislab-ccom.unh.edu");
    
    EXPECT_EQ("http",u.scheme);
    EXPECT_EQ("",u.userinfo);
    EXPECT_EQ("vislab-ccom.unh.edu",u.host);
    EXPECT_EQ(-1,u.port);
    EXPECT_EQ("",u.path);
    EXPECT_EQ("",u.query);
    EXPECT_EQ("",u.fragment);
    
    EXPECT_EQ("http://vislab-ccom.unh.edu",std::string(u));
}

TEST(URI,SimpleHTTPRootPath)
{
    gz4d::URI u("http://vislab-ccom.unh.edu/");
    
    EXPECT_EQ("http",u.scheme);
    EXPECT_EQ("",u.userinfo);
    EXPECT_EQ("vislab-ccom.unh.edu",u.host);
    EXPECT_EQ(-1,u.port);
    EXPECT_EQ("/",u.path);
    EXPECT_EQ("",u.query);
    EXPECT_EQ("",u.fragment);
    
    EXPECT_EQ("http://vislab-ccom.unh.edu/",std::string(u));
}

TEST(URI,HTTPPort)
{
    gz4d::URI u("http://vislab-ccom.unh.edu:42/");
    
    EXPECT_EQ("http",u.scheme);
    EXPECT_EQ("",u.userinfo);
    EXPECT_EQ("vislab-ccom.unh.edu",u.host);
    EXPECT_EQ(42,u.port);
    EXPECT_EQ("/",u.path);
    EXPECT_EQ("",u.query);
    EXPECT_EQ("",u.fragment);
    
    EXPECT_EQ("http://vislab-ccom.unh.edu:42/",std::string(u));
}

TEST(URI,HTTPQuery)
{
    gz4d::URI u("http://vislab-ccom.unh.edu/?what");
    
    EXPECT_EQ("http",u.scheme);
    EXPECT_EQ("",u.userinfo);
    EXPECT_EQ("vislab-ccom.unh.edu",u.host);
    EXPECT_EQ(-1,u.port);
    EXPECT_EQ("/",u.path);
    EXPECT_EQ("what",u.query);
    EXPECT_EQ("",u.fragment);
    
    EXPECT_EQ("http://vislab-ccom.unh.edu/?what",std::string(u));
}

TEST(URI,HTTPFragment)
{
    gz4d::URI u("http://vislab-ccom.unh.edu/#frag");
    
    EXPECT_EQ("http",u.scheme);
    EXPECT_EQ("",u.userinfo);
    EXPECT_EQ("vislab-ccom.unh.edu",u.host);
    EXPECT_EQ(-1,u.port);
    EXPECT_EQ("/",u.path);
    EXPECT_EQ("",u.query);
    EXPECT_EQ("frag",u.fragment);
    
    EXPECT_EQ("http://vislab-ccom.unh.edu/#frag",std::string(u));
}

TEST(URI,HTTPUser)
{
    gz4d::URI u("http://u:pwd@vislab-ccom.unh.edu/");
    
    EXPECT_EQ("http",u.scheme);
    EXPECT_EQ("u:pwd",u.userinfo);
    EXPECT_EQ("vislab-ccom.unh.edu",u.host);
    EXPECT_EQ(-1,u.port);
    EXPECT_EQ("/",u.path);
    EXPECT_EQ("",u.query);
    EXPECT_EQ("",u.fragment);
    
    EXPECT_EQ("http://u:pwd@vislab-ccom.unh.edu/",std::string(u));
}

TEST(URI,HTTPAll)
{
    gz4d::URI u("http://u:pwd@vislab-ccom.unh.edu:123/path/to/somewhere?question#fragment");
    
    EXPECT_EQ("http",u.scheme);
    EXPECT_EQ("u:pwd",u.userinfo);
    EXPECT_EQ("vislab-ccom.unh.edu",u.host);
    EXPECT_EQ(123,u.port);
    EXPECT_EQ("/path/to/somewhere",u.path);
    EXPECT_EQ("question",u.query);
    EXPECT_EQ("fragment",u.fragment);
    
    EXPECT_EQ("http://u:pwd@vislab-ccom.unh.edu:123/path/to/somewhere?question#fragment",std::string(u));
}

TEST(URI,BadPortException)
{
    EXPECT_THROW(gz4d::URI("abc://host.com:bad"), gz4d::URIException);
}

TEST(URI,Mailto)
{
    gz4d::URI u("mailto:user@example.com");
    
    EXPECT_EQ("mailto",u.scheme);
    EXPECT_EQ("",u.userinfo);
    EXPECT_EQ("",u.host);
    EXPECT_EQ(-1,u.port);
    EXPECT_EQ("user@example.com",u.path);
    EXPECT_EQ("",u.query);
    EXPECT_EQ("",u.fragment);
    
    EXPECT_EQ("mailto:user@example.com",std::string(u));
}

TEST(URI,DefaultPorts)
{
    EXPECT_EQ(80,gz4d::URI("http:").getPort());
    EXPECT_EQ(8080,gz4d::URI("http://server:8080").getPort());
    EXPECT_EQ(443,gz4d::URI("https:").getPort());
    EXPECT_EQ(-1,gz4d::URI("foo://bar/").getPort());
    EXPECT_EQ(42,gz4d::URI("foo://bar:42/").getPort());
}

TEST(URI,QueryMap)
{
    gz4d::URI::QueryMap m = gz4d::URI("http://host/path?key1=val1;key2=2").getQueryMap();
    EXPECT_EQ(2,m.size());
    EXPECT_EQ("val1",m["key1"]);
    EXPECT_EQ("2",m["key2"]);

    m = gz4d::URI("http://host/path?key1=val1&key2=2").getQueryMap();
    EXPECT_EQ(2,m.size());
    EXPECT_EQ("val1",m["key1"]);
    EXPECT_EQ("2",m["key2"]);

    m = gz4d::URI("http://host/path?key1=val1&key2=2").getQueryMap(";");
    EXPECT_EQ(1,m.size());
    EXPECT_EQ("key1",m.begin()->first);
    EXPECT_EQ("val1&key2=2",m.begin()->second);
    
    m = gz4d::URI("http://host/path?noval").getQueryMap();
    EXPECT_EQ(1,m.size());
    EXPECT_EQ("noval",m.begin()->first);
    EXPECT_EQ("",m.begin()->second);
}
