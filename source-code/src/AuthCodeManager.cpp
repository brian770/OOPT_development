#include "AuthCodeManager.hpp"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

using namespace std;

// 생성자
AuthCodeManager::AuthCodeManager()
    : authCodeList() {
    srand((unsigned int)time(NULL));
    authCodeList.emplace_back("12345", 1, 1);
}

// 인증코드 생성
string AuthCodeManager::generateCode() {
    char code_str[6]; // 숫자와 대소문자로 구성된 5자리 문자열
    int type; // 0: 숫자, 1: 소문자, 2: 대문자

    for (int i = 0; i < 5; i++) {
        type = rand() % 3; // 0, 1, 2 중 랜덤으로 선택
        if (type == 0) {
            code_str[i] = '0' + rand() % 10; // 숫자
        } else if (type == 1) {
            code_str[i] = 'a' + rand() % 26; // 소문자
        } else {
            code_str[i] = 'A' + rand() % 26; // 대문자
        }
    }

    code_str[5] = '\0'; // 문자열 종료 문자 추가

    return string(code_str); // string으로 변환
}

// 전달용 인증코드 생성
AuthCode AuthCodeManager::makeAuthCode(const string& code, int itemId, int itemNum) {
    return AuthCode(code, itemId, itemNum);
}

// 전달받은 인증코드 저장
void AuthCodeManager::saveAuthCode(const string& code, int itemId, int itemNum) {
    authCodeList.emplace_back(code, itemId, itemNum);
}

// 해당 인증코드가 존재하는지 확인
bool AuthCodeManager::isValidAuthCode(const string& code) {
    for (const auto& authCode : authCodeList) {
        if (authCode.getCode() == code) {
            return true;
        }
    }
    return false;
}

// 사용된 인증 코드 삭제
pair<int,int> AuthCodeManager::popAuthCode(const string& code) {
    for (auto iter = authCodeList.begin(); iter != authCodeList.end(); ++iter) {
        if (iter->getCode() == code) {
            AuthCode temp = *iter;
            authCodeList.erase(iter);
            return make_pair(temp.getItemId(), temp.getItemNum());
        }
    }
    return make_pair(0,0);
}


