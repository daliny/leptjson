#include "leptjson.h"
#include "gtest/gtest.h"

class leptjsontest :public ::testing::Test{
protected:
	   virtual void SetUp(){}
	   virtual void TearDown() {}
leptjson json; // 被测试的对象
};

TEST_F(leptjsontest, leptnull){
	EXPECT_EQ(LEPT_PARSE_OK, json.parse("null"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
}

TEST_F(leptjsontest, leptboolean){
	EXPECT_EQ(LEPT_PARSE_OK, json.parse("true"));
	EXPECT_EQ(LEPT_TRUE,json.gettype());
	EXPECT_EQ(LEPT_PARSE_OK, json.parse("false"));
	EXPECT_EQ(LEPT_FALSE,json.gettype());
}

TEST_F(leptjsontest, leptinvalid){
	EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.parse("tru"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
	EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.parse("n"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
	EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.parse("fase"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
}

TEST_F(leptjsontest, leptrootnotsingular){
	EXPECT_EQ(LEPT_PARSE_ROOT_NOT_SINGULAR, json.parse("false s"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
}
int main(int argc, char **argv)
{
     ::testing::InitGoogleTest(&argc, argv);
     return RUN_ALL_TESTS();
}
