#include "Bank.hpp"
#include <iostream>

using namespace std;

Bank::Bank() 
: bankId("신한은행"){
    account.insert(pair<string, int>("12345", 10000)); // test input
}

// 결제 처리 메서드
// 1: 결제 성공, 2: 카드 정보 없음, 3: 잔액 부족
int Bank::pay(const string cardNum, int cost) {
    for(auto& iter : account){
        if(iter.first == cardNum){
            if(iter.second >= cost){
                iter.second -= cost; // 잔액 차감
                return 1; // 결제 성공
            }
            else return 3; // 잔액 부족
        }
    }
    return 2;
}

map<string, int> Bank::getAccount()
{
    return account;
}
