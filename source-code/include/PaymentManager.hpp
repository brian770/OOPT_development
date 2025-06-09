#ifndef PaymentManager_HPP
#define PaymentManager_HPP

#include "Bank.hpp"
#include <string>

class PaymentManager {
private:
    Bank* bank;
public:
    // 생성자
    PaymentManager(Bank* b);

    // 카드번호를 가져오는 메서드
    std::string getCardNum();

    // 결제를 요청하는 메서드 (카드번호를 내부적으로 호출해서 사용)
    // 1: 성공, 2: 카드 정보 안 맞음, 3: 계좌에 돈이 부족함, -1: 예외상황 발생
    int requestPayment(int cost);
};

#endif // PaymentManager_HPP