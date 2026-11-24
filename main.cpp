#include <bits/stdc++.h>
using namespace std;

enum class Side{ 
    Buy,
    Sell
};
struct Order {
    int id;
    Side side;
    long long price;
    int quantity;
};
struct Trade {
    int buyOrderId;
    int sellOrderId;
    long long price;
    int quantity;
};
void printOrder(const Order&o){
    long long whole=o.price/100;
    int paise=o.price%100;
    string paise_str=to_string(paise);
    if(paise<10){
        paise_str='0'+paise_str;
    }
    string value=to_string(whole)+'.'+paise_str;
    cout<<"#"<<o.id<<' '<<(o.side==Side::Buy?"Buy":"Sell")<<' '<<o.quantity<<' '<<'@'<<' '<<value<<'\n';
}
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
}

