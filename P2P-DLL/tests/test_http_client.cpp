#include <gtest/gtest.h>
#include "../include/HttpClient.h"
#include "../include/Logger.h"
#include <thread>
#include <chrono>

using namespace P2P;

class HttpClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for tests
        Logger::GetInstance().Initialize(LogLevel::DEBUG, "", false);
    }
    
    void TearDown() override {
        Logger::GetInstance().Shutdown();
    }
};

TEST_F(HttpClientTest, DefaultConstruction) {
    HttpClient client;
    EXPECT_FALSE(client.IsConfigured());
}

TEST_F(HttpClientTest, SetBaseUrl) {
    HttpClient client;
    client.SetBaseUrl("http://localhost:8000");
    EXPECT_TRUE(client.IsConfigured());
}

TEST_F(HttpClientTest, SetAuthToken) {
    HttpClient client;
    client.SetBaseUrl("http://localhost:8000");
    client.SetAuthToken("test_token_12345");
    EXPECT_TRUE(client.IsConfigured());
}

TEST_F(HttpClientTest, SetTimeout) {
    HttpClient client;
    client.SetBaseUrl("http://localhost:8000");
    client.SetTimeout(5000);
    EXPECT_TRUE(client.IsConfigured());
}

TEST_F(HttpClientTest, GetRequestNotConfigured) {
    HttpClient client;
    auto response = client.Get("/test");
    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.error_message, "Client not configured");
}

TEST_F(HttpClientTest, PostRequestNotConfigured) {
    HttpClient client;
    auto response = client.Post("/test", "{}");
    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.error_message, "Client not configured");
}

TEST_F(HttpClientTest, PutRequestNotConfigured) {
    HttpClient client;
    auto response = client.Put("/test", "{}");
    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.error_message, "Client not configured");
}

TEST_F(HttpClientTest, DeleteRequestNotConfigured) {
    HttpClient client;
    auto response = client.Delete("/test");
    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.error_message, "Client not configured");
}

// Note: The following tests require a running HTTP server
// They are disabled by default and can be enabled for integration testing

TEST_F(HttpClientTest, DISABLED_GetRequestSuccess) {
    HttpClient client;
    client.SetBaseUrl("http://httpbin.org");
    
    auto response = client.Get("/get");
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status_code, 200);
    EXPECT_FALSE(response.body.empty());
}

TEST_F(HttpClientTest, DISABLED_PostRequestSuccess) {
    HttpClient client;
    client.SetBaseUrl("http://httpbin.org");
    
    auto response = client.Post("/post", R"({"test": "data"})");
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status_code, 200);
    EXPECT_FALSE(response.body.empty());
}

TEST_F(HttpClientTest, DISABLED_PutRequestSuccess) {
    HttpClient client;
    client.SetBaseUrl("http://httpbin.org");
    
    auto response = client.Put("/put", R"({"test": "data"})");
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status_code, 200);
    EXPECT_FALSE(response.body.empty());
}

TEST_F(HttpClientTest, DISABLED_DeleteRequestSuccess) {
    HttpClient client;
    client.SetBaseUrl("http://httpbin.org");
    
    auto response = client.Delete("/delete");
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status_code, 200);
    EXPECT_FALSE(response.body.empty());
}

TEST_F(HttpClientTest, DISABLED_AuthenticationHeader) {
    HttpClient client;
    client.SetBaseUrl("http://httpbin.org");
    client.SetAuthToken("test_bearer_token");
    
    auto response = client.Get("/headers");
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status_code, 200);
    EXPECT_NE(response.body.find("Bearer test_bearer_token"), std::string::npos);
}

TEST_F(HttpClientTest, DISABLED_TimeoutHandling) {
    HttpClient client;
    client.SetBaseUrl("http://httpbin.org");
    client.SetTimeout(1); // 1ms timeout - should fail
    
    auto response = client.Get("/delay/5");
    EXPECT_FALSE(response.success);
    EXPECT_FALSE(response.error_message.empty());
}

TEST_F(HttpClientTest, DISABLED_InvalidUrl) {
    HttpClient client;
    client.SetBaseUrl("http://invalid-domain-that-does-not-exist-12345.com");
    
    auto response = client.Get("/test");
    EXPECT_FALSE(response.success);
    EXPECT_FALSE(response.error_message.empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

