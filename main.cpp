/*
 * Vecrison 0.1 08.06.20
*/
#include <iostream>
#include <QVector>
#include <QDebug>
#include <iomanip>
#include <string>
#include <sstream>

//#include <stdint.h>

//#define ROTL8(x,shift) ((uint8_t) ((x) << (shift)) | ((x) >> (8 - (shift))))

//void initialize_aes_sbox(uint8_t sbox[256]) {
//    uint8_t p = 1, q = 1;

//    /* loop invariant: p * q == 1 in the Galois field */
//    do {
//        /* multiply p by 3 */
//        p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

//        /* divide q by 3 (equals multiplication by 0xf6) */
//        q ^= q << 1;
//        q ^= q << 2;
//        q ^= q << 4;
//        q ^= q & 0x80 ? 0x09 : 0;

//        /* compute the affine transformation */
//        uint8_t xformed = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);

//        sbox[p] = xformed ^ 0x63;
//    } while (p != 1);

//    /* 0 is a special case since it has no inverse */
//    sbox[0] = 0x63;
//}

//QChar toChar(int n) {
//  const QString alpha = "0123456789ABCDEF";
//  return alpha.at(n);
//}
//QString toHex(int d) {
//  int r = d % 16;
//  if (d - r == 0) {
//    return toChar(r);
//  }
//  return toHex( (d - r)/16 ) + toChar(r);
//}
QString toHex(int n) {
    std::stringstream stream;
    stream << std::hex << n;
    std::string result(stream.str());
    return QString::fromStdString(result);
}

int toDec(QString str) {
    int n;
    std::istringstream(str.toStdString()) >> std::hex >> n;
    return n;
}

QVector<QVector<int>> subBytes(QVector<QVector<int>> src, const QVector<QVector<int>> box, const QString numInHex) {
    QVector<QVector<int>> cpy = src;
    for (int i(0); i < src.size(); i++) {
        for (int j(0); j < src[i].size(); j++) {
            QString strHex(toHex(src[i][j]));
            if (strHex.size() > 1) {
                cpy[i][j] = box[numInHex.indexOf(strHex[0])][numInHex.indexOf(strHex[1])];
            } else {
                cpy[i][j] = box[0][numInHex.indexOf(strHex[0])];
            }
        }
    }
    return cpy;
}

QVector<QVector<int>> getState(const QVector<int> vec, int Nb) {
    QVector<QVector<int>> out;
    for(int j(0); j < Nb; j++) {
        QVector<int> temp;
        for(int i(j); i < vec.length(); i += 4) {
            temp.append(vec[i]);
        }
        out.append(temp);
        temp.clear();
    }
    return out;
}

QVector<QVector<int>> shiftRow(const QVector<QVector<int>> src) {
    QVector<QVector<int>> cpy = src;
    for (int i(1); i < src.size(); i++) {
        for(int j(0); j < src[i].size(); j++) {
            cpy[i][j] = src[i][(j + i) % src[i].size()];
        }
    }
    return cpy;
}

int mul_bytes(int a, int b)
{
  /*проход по каждому биту*/
  int c = 0, mask = 1, bit, cpyA;
  int i, j;
  for (i = 0; i < 8; i++) {
    bit = b & mask;
    if (bit) {
      cpyA = a;
      for (j = 0; j < i; j++) {
        cpyA = cpyA << 1;
        if (cpyA > 0xff) {
          cpyA = cpyA - 0xff;
        }
      }
      c = c ^ cpyA;
    }
    b = b >> 1;
  }
  return c;
}

QVector<QVector<int>> mixColums(const QVector<QVector<int>> src,const QVector<QVector<int>> a) {
    QVector<QVector<int>> cpy = src;
    for (int i(0); i < a.size(); i++) {
        for (int k(0); k < a[i].size(); k++) {
            int temp(0);
            for (int j(0); j < a[0].size(); j++) {
                QString strHex(toHex(src[j][i]));
                QString strHexA(toHex(a[k][j]));
                if(a[k][j] == 1) {
                    temp ^= src[j][i];
                } else {
                    temp ^= mul_bytes(a[k][j], src[j][i]);
                }
            }
            if (temp > 0xff) {
                temp = temp - 0xff - 1;
            }
            cpy[k][i] = temp;
        }
    }
    return cpy;
}

QVector<QVector<int>> addRoundKey(QVector<QVector<int>> inp,  QVector<QVector<int>> oldKey) {
    QVector<QVector<int>> cpy = inp;
    for (int i(0); i < inp.size(); i++) {
        for (int j(0); j < inp.size(); j++) {
            cpy[i][j] ^= oldKey[i][j];
        }
    }
    return cpy;
}

QList<QVector<QVector<int>>> expKey(const QVector<QVector<int>> key, const QVector<QVector<int>> rcon, const QVector<QVector<int>> box, const QString numInHex, const int Nr) {

    QList<QVector<QVector<int>>> out;
    QVector<QVector<int>> tempKey;
    for(int i(0); i < key.size(); i++) {
        QVector<int> temp;
        temp<<key[0][i]<<key[1][i]<<key[2][i]<<key[3][i];
        tempKey.append(temp);
    }
    for(int i(0); i < Nr; i++){
        QVector<QVector<int>> oldKey = tempKey;
        tempKey.last().append(tempKey.last()[0]);
        tempKey.last().pop_front();
        for (int j(0); j < tempKey.last().size(); j++) {
            QString strHex(toHex(tempKey.last()[j]));
            if (strHex.size() > 1) {
                tempKey.last()[j] = box[numInHex.indexOf(strHex[0])][numInHex.indexOf(strHex[1])];
            } else {
                tempKey.last()[j] = box[0][numInHex.indexOf(strHex[0])];
            }
        }
        tempKey.last()[0] ^= rcon[i][0];
        tempKey.first() = tempKey.last();

        for (int i(0); i < oldKey.first().size(); i++) {
            tempKey.first()[i] ^= oldKey.first()[i];
        }

        for (int i(1); i < oldKey.first().size(); i++) {
            for (int j(0); j < oldKey.first().size(); j++) {
                tempKey[i][j] = tempKey[i-1][j] ^ oldKey[i][j];
            }
        }
        QVector<QVector<int>> convertMat;
        for(int i(0); i < tempKey.size(); i++) {
            QVector<int> temp;
            temp<<tempKey[0][i]<<tempKey[1][i]<<tempKey[2][i]<<tempKey[3][i];
            convertMat.append(temp);
        }
        out.append(convertMat);
    }
    return out;

}

QVector<QVector<int>> invShiftRow(const QVector<QVector<int>> src) {
    QVector<QVector<int>> out = src;
    for (int i(1); i < src.size(); i++) {
        for(int j(0); j < i; j++){
            out[i].push_front(out[i].last());
            out[i].pop_back();
        }
    }
    return out;
}

QString showVector(const QVector<QVector<int>> src) {
    QString out;
    for(int i(0); i < src.size(); i++) {
        for(int j(0); j < src.size(); j++) {
            out.append(toHex(src[j][i]));
        }
    }
    return out;
}

int main() {
    const QString numInHex = "0123456789abcdef";
    const QVector<QVector<int>> sbox = {
        {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
        {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
        {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
        {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
        {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
        {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
        {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
        {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
        {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
        {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
        {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
        {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
        {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
        {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
        {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
        {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}};
    const QVector<QVector<int>> inv_sbox = {
        {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb},
        {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb},
        {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e},
        {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25},
        {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92},
        {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84},
        {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06},
        {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b},
        {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73},
        {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e},
        {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b},
        {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4},
        {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f},
        {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef},
        {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61},
        {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d}};
//    const QVector<int> key = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
//    const QVector<int> input = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    const QVector<int> input = {0x00, 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    const QVector<int> key = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    const QVector<QVector<int>> a = {
        {0x02,0x03,0x01,0x01},
        {0x01,0x02,0x03,0x01},
        {0x01,0x01,0x02,0x03},
        {0x03,0x01,0x01,0x02}};
    const QVector<QVector<int>> a_inv = {
        {0x0e,0x0b,0x0d,0x09},
        {0x09,0x0e,0x0b,0x0d},
        {0x0d,0x09,0x0e,0x0b},
        {0x0b,0x0d,0x09,0x0e}};

    const QVector<QVector<int>> rcon = {
        {0x01, 0x00, 0x00, 0x00},
        {0x02, 0x00, 0x00, 0x00},
        {0x04, 0x00, 0x00, 0x00},
        {0x08, 0x00, 0x00, 0x00},
        {0x10, 0x00, 0x00, 0x00},
        {0x20, 0x00, 0x00, 0x00},
        {0x40, 0x00, 0x00, 0x00},
        {0x80, 0x00, 0x00, 0x00},
        {0x1b, 0x00, 0x00, 0x00},
        {0x36, 0x00, 0x00, 0x00},
    };

    const int Nb(4);
    int Nr = 0;
    if(key.length()*8 == 128){
        Nr = 10;
    } else if(key.length()*8 == 196) {
        Nr = 12;
    } else if (key.length()*8 == 256) {
        Nr = 14;
    } else {
        qDebug()<<"Wrong key length";
        return 0;
    }
    QVector<QVector<int>> stateInput(getState(input, Nb));
    QVector<QVector<int>> stateKey(getState(key, Nb));
    QList<QVector<QVector<int>>> roundsKey = expKey(stateKey, rcon, sbox, numInHex, Nr);
    QVector<QVector<int>> test;
    qDebug()<<0<<"input:"<<showVector(stateInput);
    qDebug()<<0<<"k_sch:"<<showVector(stateKey);
    test = addRoundKey(stateInput, stateKey);
    for (int i(0); i < Nr-1; i++) {
        qDebug()<<i<<"start:"<<showVector(test);
        test = subBytes(test, sbox, numInHex);
        qDebug()<<i<<"s_box:"<<showVector(test);
        test = shiftRow(test);
        qDebug()<<i<<"s_row:"<<showVector(test);
        test = mixColums(test, a);
        qDebug()<<i<<"m_col:"<<showVector(test);
        qDebug()<<i<<"k_sch:"<<showVector(roundsKey[i]);
        test = addRoundKey(test, roundsKey[i]);
    }
//    qDebug()<<"***********************************************************";
    qDebug()<<Nr<<"start:"<<showVector(test);
    test = subBytes(test, sbox, numInHex);
    qDebug()<<Nr<<"s_box:"<<showVector(test);
    test = shiftRow(test);
    qDebug()<<Nr<<"s_row:"<<showVector(test);
    qDebug()<<Nr<<"k_sch:"<<showVector(roundsKey.last());
    test = addRoundKey(test, roundsKey.last());
    qDebug()<<Nr<<"output:"<<showVector(test);
    qDebug()<<"******************/////////////////***********************";


    qDebug()<<"input    :"<<showVector(test);
    qDebug()<<"k_sch :"<<showVector(roundsKey.last());
    QVector<QVector<int>> decrip;
    decrip = addRoundKey(test, roundsKey.last());
//    qDebug()<<Nr<<":"<<showVector(decrip);
    for (int i(Nr-2); i >= 0; i--) {
        qDebug()<<"start    :"<<i<<showVector(decrip);
        decrip = invShiftRow(decrip);
        qDebug()<<"is_row   :"<<i<<showVector(decrip);
        decrip = subBytes(decrip, inv_sbox, numInHex);
        qDebug()<<"is_box :"<<i<<showVector(decrip);
        qDebug()<<"key      :"<<i<<showVector(roundsKey[i]);
        decrip = addRoundKey(decrip, roundsKey[i]);
        qDebug()<<"is_add   :"<<i<<showVector(decrip);
        decrip = mixColums(decrip, a_inv);
//        decrip = InvMixColumns(decrip);
//        qDebug()<<"mixColumn:"<<i<<showVector(decrip);
    }
    qDebug()<<"***********************************************************";
    qDebug()<<"start    :"<<0<<showVector(decrip);
    decrip = invShiftRow(decrip);
    qDebug()<<"is_row   :"<<0<<showVector(decrip);
    decrip = subBytes(decrip, inv_sbox, numInHex);
    qDebug()<<"is_box :"<<0<<showVector(decrip);
    decrip = addRoundKey(decrip, stateKey);
    qDebug()<<"is_add   :"<<0<<showVector(decrip);
    qDebug()<<0<<":"<<showVector(decrip);
//    for(int i(0); i < roundsKey.size(); i++){
//        qDebug()<<showVector(roundsKey[i]);
//    }
    return 0;
}
