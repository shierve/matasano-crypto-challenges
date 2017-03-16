#include "Output.hh"
#include "File.hh"
#include "Aes.hh"
#include "Stream.hh"
#include "Conversion.hh"
#include "User.hh"
#include "Hash.hh"
#include "Data.hh"

void ch25(){
    vector<byte> v = fetchFromFile("INPUT/ch7.txt");
    v = aes_128_ECB_de(v, keySub);
    vector<byte> nonce = intToByteArray(12530, false);
    v = aes_128_CTR(v, keySub, nonce);
    vector<byte> plain;

    for(int i = 0; i < v.size(); ++i){
        bool f = false;
        vector<byte> letter;
        letter.push_back(0);
        for(int b = 0; b < 256 and not f; ++b){
            letter[0] = b;
            if(v == edit_CTR(v, keySub, nonce, i, letter)){
               f = true;
               plain.push_back(b);
               cout << (char)b;
            }
        }
    }

    // plain has the unencrypted content
    cout << endl;
}

void ch26(){
    string obj = ":admin<true:";

    vector<byte> v = User::encryptData_CTR(obj);

    vector<byte> mask (v.size(), 0);
    mask[32] = 1;
    mask[32 + 6] = 1;
    mask[32 + 11] = 1;
    v = fixed_xor(v, mask);

    if(User::searchString_CTR(v)) cout << "admin" << endl;
    else cout << "not admin" << endl;
}

void ch27(){
    //encrypt url string with User (3 blocks)
    string s = "www.aaaaaaaaaaaa.com/aaaaaaaaaaaaaaa/aaaaaaaaaaa";
    vector<byte> v = User::encryptData(s);
    //modify the message C1, 0, C1
    for(int i = 16; i < 32; ++i){
        v[i] = 0;
        v[i + 16] = v[i - 16];
    }
    //key: P1' xor P3'
    v = User::check_ascii(v);
    if(v.empty())
        cout << "good ascii" << endl;
    else{
        vector<byte> k = block_xor(&v[0], &v[32]);
        cout << "key found: ";
        printHex(k);
    }
}

void ch28(){
    string message = " brown fox jumps over the lazy dog";
    string key = "The quick";
    vector<byte> m = stringToByteArray(message);
    vector<byte> k = stringToByteArray(key);
    vector<byte> mac = sha1_mac(k, m);
    printHex(mac);
}

void ch29(){
    string message = "comment1=cooking%20MCs;userdata=foo;comment2=%20like%20a%20pound%20of%20bacon";
    vector<byte> m = stringToByteArray(message);
    vector<byte> k = gen_random_bytes(-1);
    cout << "ks: " << k.size() << endl;
    vector<byte> mac = sha1_mac(k, m);
    printHex(mac);
    //hash additional data
    uint32_t h0, h1, h2, h3, h4;
    h0 = uint32_from_array(&mac[0]);
    h1 = uint32_from_array(&mac[4]);
    h2 = uint32_from_array(&mac[8]);
    h3 = uint32_from_array(&mac[12]);
    h4 = uint32_from_array(&mac[16]);
    vector<byte> new_message = stringToByteArray(";admin=true");
    //1024 will be correct for not too big keysizes
    uint64_t mm = 1024 + new_message.size()*8;
    vector<byte> new_mac = sha1(new_message, h0, h1, h2, h3, h4, mm);
    printHex(new_mac);
    //try different key lengths to forge the message: (message|padding|new)
    bool found = false;
    for(int i = 0; i < 100 and not found; ++i){
        vector<byte> forged_m = append_arrays(m, sha1_glue_padding(i, m.size()));
        forged_m = append_arrays(forged_m, new_message);
        //printChar(forged_m);
        //cout << endl;
        vector<byte> comp = sha1_mac(k, forged_m);
        if(comp == new_mac){
            found = true;
            cout << "keysize: " << i << endl;
            printChar(forged_m);
            cout << endl;
        }
    }
    if(not found) cout << "not found" << endl;
}

void ch30(){
    string st = "message digest";
    vector<byte> v = stringToByteArray(st);
    v = md4(v);
    printHex(v);
    cout << "------------------------" << endl;

    string message = "comment1=cooking%20MCs;userdata=foo;comment2=%20like%20a%20pound%20of%20bacon";
    vector<byte> m = stringToByteArray(message);
    vector<byte> k = gen_random_bytes(-1);
    cout << "ks: " << k.size() << endl;
    vector<byte> mac = md4_mac(k, m);
    printHex(mac);
    //hash additional data
    uint32_t a, b, c, d;
    a = uint32_from_array(&mac[0]);
    b = uint32_from_array(&mac[4]);
    c = uint32_from_array(&mac[8]);
    d = uint32_from_array(&mac[12]);
    vector<byte> new_message = stringToByteArray(";admin=true");
    //1024 will be correct for not too big keysizes
    uint64_t mm = 1024 + new_message.size()*8;
    vector<byte> new_mac = md4(new_message, a, b, c, d, mm);
    printHex(new_mac);
    //try different key lengths to forge the message: (message|padding|new)
    bool found = false;
    for(int i = 0; i < 100 and not found; ++i){
        vector<byte> forged_m = append_arrays(m, md4_glue_padding(i, m.size()));
        forged_m = append_arrays(forged_m, new_message);
        //printChar(forged_m);
        //cout << endl;
        vector<byte> comp = md4_mac(k, forged_m);
        if(comp == new_mac){
            found = true;
            cout << "keysize: " << i << endl;
            printChar(forged_m);
            cout << endl;
        }
    }
    if(not found) cout << "not found" << endl;
}

int main(){
    ch30();
}
