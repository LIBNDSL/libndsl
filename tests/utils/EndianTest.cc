////
// @file EndianTest.cc
// @brief
// 测试Endian
//
// @author lanyue
// @email luckylanry@163.com 
//

#include "../catch.hpp"

#include "ndsl/utils/Endian.h"


TEST_CASE("utils/Endian")
{   
    using namespace std;
    ndsl::utils::Endian e;

    SECTION("hToN64"){
        e.hToN64(78);
         
    }
     SECTION("hToN32"){
        int a=12;
        e.hToN32(a);
       
    }
     SECTION("hToN16"){
        uint16_t u16t = 13;
        e.hToN16(u16t);
    }

    SECTION("nToH64"){
        int b=67;
        e.nToH64(b);
    }
     SECTION("nToH32"){
         int c = 68;
         e.nToH32(c);
         // cout << ret << endl;
    }
     SECTION("nToH16"){
         int d= 69;
         e.hToN16(d);
         // cout << ret << endl;
    }
    

}  