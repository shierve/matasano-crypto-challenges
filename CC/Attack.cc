#include "Attack.hh"


byte Attack::testBytes(const vector<byte> v, int thres){
    vector<byte> b;
    byte key = 0;
    int best = 0;
    for(int i = 0; i < 256; ++i){
        b = Xor::single_key_xor(v, (byte) i);
        int aux = Analysis::frequency_evaluation(b);
        if(aux > best){
            best = aux;
            key = (byte)i;
        }
        if(aux >= thres){
            cout << "(" << aux << ")";
            Output::printChar(b);
            cout << " [Key: " << i << "|ASCII:'";
            Output::printAscii(i);
            cout <<"']" << endl;
        }
    }
    cout << "best: " << (int)key << endl;
    return key;
}

vector<byte> Attack::findRepeatingKey(const vector< vector<byte> >& groupedBlock){
    vector<byte> best_key (groupedBlock.size(), 0);
    for(int i = 0; i < groupedBlock.size(); ++i){
        cout << endl << "byte " << i << ":" << endl << endl;
        best_key[i] = testBytes(groupedBlock[i], THRES);
    }
    return best_key;
}

void Attack::byte_at_a_time(vector<byte> (*f)(const vector<byte>&)){
    //block size
    int blockSize;
    {
        int firstSize;
        vector<byte> enc (1, 65);
        enc = f(enc);
        firstSize = enc.size();
        int c = 1;
        while(firstSize == enc.size()){
            vector<byte> inj (c, 65);
            enc = f(inj);
            ++c;
        }
        blockSize = enc.size() - firstSize; //int blockSize
    }
    cout << "Block size: " << blockSize * 8 << "-bit" << endl;
    //prepend size
    int prependSize;
    {
        vector<byte> first (blockSize, 0);
        first = f(first);
        vector<byte> shiftable(blockSize, 0);
        Block::shiftBytes(shiftable, 1);
        vector<byte> aux = f(shiftable);
        int b2; //second occupied block (or unic) counting from 0
        bool found = false;
        for(int i = 0; i < aux.size() and not found; i += blockSize){
            if(!Block::compareBlocks(&first[i], &aux[0], blockSize)){
                b2 = i;
                found = true;
            }
        }
        
        found = false;
        for(int i = 0; i < shiftable.size() - 1 and not found; ++i){
            //AAAABA||AAABAA ...
            Block::shiftBytes(shiftable, 0);
            aux = f(shiftable);
            if(Block::compareBlocks(&first[b2], &aux[b2], blockSize)){
                prependSize = i + 1;
                found = true;
            }
        }
        if(not found) prependSize = 0;
    }
    cout << "Prepend size: " << prependSize << " bytes" << endl;
    //append size
    int appendSize;
    {
        vector<byte> inj (blockSize - prependSize, 65); //assumes append size is smaller than blockSize
        inj = f(inj);
        appendSize = inj.size() - blockSize;
    }
    cout << "Append size: " << appendSize << " bytes" << endl;
    //ecb check
    if(!Analysis::mode_detector(f)){
        //cout << "function not encrypting in ECB mode" << endl;
        //return;
    }

    //start
    vector<byte> inj ((blockSize - prependSize) + appendSize, 65);
    for(int i = 0; i < appendSize; ++i){
        vector<byte> test ((blockSize - prependSize) + appendSize - (1 + i), 65); //block - pre = block (pre = 0)
        test = f(test);
        bool found = false;
        for(int j = 0; j < 256 and not found; ++j){
            inj[inj.size() - 1] = j;
            vector<byte> injEnc = f(inj);
            if(Block::compareBlocks(&injEnc[appendSize], &test[appendSize], blockSize)) //here
                found = true;
        }
        Block::shiftBytes(inj,0);
    }
    //append is in inj, starting from the pos bS - pS - 1
    for(int i = 0; i < (blockSize - prependSize) - 1; ++i){
        Block::shiftBytes(inj,0);
    }
    inj = Block::copyFrom(&inj[0], appendSize);
    cout << endl << "Append found!" << endl << "----------------------" << endl;
    Output::printChar(inj);
    cout << endl;
}

vector<byte> Attack::padding_oracle_attack(const vector<byte>& c, const vector<byte> iv, Target tgt){
    vector<byte> plain (c.size(), 'X');
    vector<byte> b1 (16);
    vector<byte> b2 (16);
    //first block
    b1 = iv;
    for(int i = 0; i < 16; ++i) b2[i] = c[i];
    b1 = break_block(b1,b2,tgt);
    for(int i = 0; i < 16; ++i)
        plain[i] = b1[i];
    //other blocks
    for(int block = 1; block < c.size()/16; ++block){
        for(int i = 0; i < 16; ++i){
            b1[i] = c[(block*16) - 16 + i];
            b2[i] = c[(block*16) + i];
        }
        b1 = break_block(b1,b2,tgt);
        for(int i = 0; i < 16; ++i)
            plain[(block*16) + i] = b1[i];
    }
    return plain;
}

vector<byte> Attack::break_block(const vector<byte>& b1, const vector<byte>& b2, Target tgt){
    vector<byte> bytes(16, '.');
    //last byte
    vector<byte> simIV = b1;
    bool found = false;
    for(int d = 0; d < 256 and not found; ++d){
        simIV[15] = b1[15] xor d xor 1;
        if(tgt.padding_oracle(b2, simIV)){
            simIV[14] = simIV[14] xor 255;
            if(tgt.padding_oracle(b2, simIV)){
                bytes[15] = (byte)d;
                found = true;
            }
            simIV[14] = simIV[14] xor 255;
        }
    }
    //last byte found
    for(int by = 14; by >= 0; --by){
        simIV = b1;
        byte pad = 16 - by;
        found = false;
        for(int d = 0; d < 256 and not found; ++d){
            for(int i = 15; i > by; --i)
                simIV[i] = b1[i] xor bytes[i] xor pad;
            simIV[by] = b1[by] xor d xor pad;
            if(tgt.padding_oracle(b2, simIV)){
                bytes[by] = (byte)d;
                found = true;
            }
        }
    }
    return bytes;
}







