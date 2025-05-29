#pragma once
#include "Bank.hpp"
#include "ItemManager.hpp"
#include "PaymentManager.hpp"
#include "AuthCodeManager.hpp"
#include "AltDVMManager.hpp"
#include "MsgManager.hpp"
#include "P2PServer.hpp"
#include <string>

class DVM {
private:
    std::string DVMId;
    int coorX;
    int coorY;
    int port;

    Bank bank;
    ItemManager itemManager;
    PaymentManager paymentManager;
    AuthCodeManager authCodeManager;
    AltDVMManager altDVMManager;

    P2PClient p2pClient;
    MsgManager msgManager;

    P2PServer p2pServer;

public:
    DVM(const std::string& id, int x, int y, int port); // 생성자
    void run(); // 프로그램 메인 흐름

private:
    std::string askBuyOrCodeInput();
    std::string askUserPrepayment();
    std::pair<int, int> requestSelect();
    void showPaymentResult(int payResult);
    void showPrepaymentResult(const std::string& authCode, std::pair<int,int> location);
    std::string requestAuthCode();

    void handleBuyFlow();
    void handleAuthCodeFlow();

};
