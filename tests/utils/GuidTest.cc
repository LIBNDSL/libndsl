////
// @file GuidTest.cc
// @brief
// Guid测试
//
// @author why
// @email 136046355@qq.com
//
#include "../catch.hpp"
#include "ndsl/utils/Guid.h"
#include "ndsl/utils/Log.h"

TEST_CASE("Guid"){
	
	ndsl::utils::Guid g1,g2;
	
	SECTION("generate"){
		REQUIRE(g1.generate() == 0);
	}
	SECTION("toString"){
		char str[32];
		g1.generate();
		g1.toString(str);
//		std::cout << str << std::endl;
		REQUIRE(g1.toString(str) == 0);
	}
	SECTION("toGuid_t"){
		char str[33] = "0D1A1E81BA3540B493340D84B61775E2";		
		g1.toGuid_t(str);	
		REQUIRE(g1.toGuid_t(str) == 0);
	}
	SECTION("operator=="){
		char str[33] = "0D1A1E81BA3540B493340D84B61775E2";		
		g1.toGuid_t(str);
		char str2[33] = "0D1A1E81BA3540B493340D84B61775E2";		
		g2.toGuid_t(str2);
		REQUIRE((g1 == g2) == true);
	}
	SECTION("operator<"){
		char str[33] = "0D1A1E81BA3540B493340D84B61775E2";		
		g1.toGuid_t(str);
		char str2[33] = "A000277BA39B4384AFC4A05CE8CFA6DD";		
		g2.toGuid_t(str2);
		REQUIRE((g1 < g2) == true);
	}
	SECTION("operator>"){
		char str[33] = "0D1A1E81BA3540B493340D84B61775E2";		
		g1.toGuid_t(str);
		char str2[33] = "A000277BA39B4384AFC4A05CE8CFA6DD";		
		g2.toGuid_t(str2);
		REQUIRE((g2 > g1) == true);
	}
}
