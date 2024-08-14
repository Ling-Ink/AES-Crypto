#include <iostream>
#include <iomanip> 
#include <string>

#include "base64.h"
#include <bitset>

const int N = 8; // length of the key in 32-bit words, 8 words for AES-256
const int Nr = N + 6; // Number round
const int Nw = 4 * (Nr + 1); // Number W size

/** 密钥扩展数组 */
int w[Nw] = { 0 };
/** S盒 */
const unsigned char s_box[16][16] = {
    0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76,
    0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0,
    0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15,
    0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75,
    0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84,
    0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF,
    0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8,
    0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2,
    0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73,
    0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB,
    0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79,
    0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08,
    0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A,
    0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E,
    0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF,
    0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16
};

/** 数据转换 */
int     convertChrToInt(char* str) {
    return ((int)str[0] & 0x000000ff) << 24 // 24
        |  ((int)str[1] & 0x000000ff) << 16 // 16
        |  ((int)str[2] & 0x000000ff) << 8  // 8
        |  ((int)str[3] & 0x000000ff);
}
void    splitIntToArray(int num, int array[4]) {
    array[0] = num >> 24    & 0x000000ff;
    array[1] = num >> 16    & 0x000000ff;
    array[2] = num >> 8     & 0x000000ff;
    array[3] = num          & 0x000000ff;
}
int     mergeArrayToInt(int array[4]) {
    return (array[0] << 24)
        |  (array[1] << 16)
        |  (array[2] << 8 )
        |  (array[3]);
}
void    splitChrTo4xMatrix(char* str, int m[4]) {
    for (int i = 0; i < 16; i += 4)
        m[i / 4] = convertChrToInt(str + i);
}
void    merge4xMatrixToChr(int m[4], char* str) {
    int mArray[4] = { 0 };
    for (int i = 0; i < 4; i++) {
        splitIntToArray(m[i], mArray);
        for (int j = 0; j < 4; j++) 
            *str++ = (char)mArray[j];
    }
}

void    dbg_dumpW(int w[44]) {
    std::cout << "= > dbg_dumpW" << std::endl;
    std::cout << "= > w[0] = " << w[0] << std::endl;
    int col0[Nw], col1[Nw], col2[Nw], col3[Nw];
    for (int i = 0; i < Nw; i++) {
        int row[4] = { 0 };
        splitIntToArray(w[i], row);
        col0[i] = row[0];
        col1[i] = row[1];
        col2[i] = row[2];
        col3[i] = row[3];
    }
    std::cout << std::hex; /** 启用十六进制输出 */
    for (int i = 0; i < Nw; i++)
        std::cout << std::setw(2) << std::setfill('0') << col0[i] << " ";
    std::cout << std::endl;
    for (int i = 0; i < Nw; i++)
        std::cout << std::setw(2) << std::setfill('0') << col1[i] << " ";
    std::cout << std::endl;
    for (int i = 0; i < Nw; i++)
        std::cout << std::setw(2) << std::setfill('0') << col2[i] << " ";
    std::cout << std::endl;
    for (int i = 0; i < Nw; i++)
        std::cout << std::setw(2) << std::setfill('0') << col3[i] << " ";
    std::cout << std::dec << std::endl; /** 停用十六进制输出 */
}
void    dbg_dumpMatrix(int m[4]) {
    std::cout << "= > dbg_dumpMatrix" << std::endl;
    int matrix[4][4] = { 0 };
    for (int i = 0; i < 4; i++)
        splitIntToArray(m[i], matrix[i]);
    std::cout << std::hex; /** 启用十六进制输出 */
    for (int i = 0; i < 4; i++) { /* 列 */
        for (int j = 0; j < 4; j++) { /* 行 */
            std::cout << std::setw(2) << std::setfill('0') << matrix[j][i] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::dec; /** 停用十六进制输出 */
}
void    dbg_dumpMatrix(int matrix[4][4]) {
    std::cout << "= > dbg_dumpMatrix" << std::endl;
    std::cout << std::hex; /** 启用十六进制输出 */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << std::setw(2) << std::setfill('0') << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::dec; /** 停用十六进制输出 */
}
void    dbg_dumpCol(int m) {
    std::cout << "= > dbg_dumpCol" << std::endl;
    int wArray[4] = {0};
    splitIntToArray(m, wArray);
    std::cout << std::hex; /** 启用十六进制输出 */
    for (int i = 0; i < 4; i++)
        std::cout << std::setw(2) << std::setfill('0') << wArray[i] << " ";
    std::cout << std::dec << std::endl; /** 停用十六进制输出 */
}
void    dbg_splitArrayTo4xMatrix(int m[4], int matrix[4][4]) {
    for (int i = 0; i < 4; i++)
        splitIntToArray(m[i], matrix[i]);
}

/** 字循环 */
int     RotWord(int m) {
    int mArray[4] = { 0 };
    splitIntToArray(m, mArray);
    for (int i = 3; i > 0; i--)
        mArray[0] ^= mArray[i] ^= mArray[0] ^= mArray[i];
    return mergeArrayToInt(mArray);
}
/** 从S盒中取对应值 */
int     getNumFromSBox(int num) {
    int row = (num & 0x000000f0) >> 4;
    int col = num & 0x0000000f;
    return s_box[row][col];
}
/** 用S盒替换一列W值 */
int     SubWord(int m) {
    int mArray[4] = { 0 };
    splitIntToArray(m, mArray);
    for (int i = 0; i < 4; i++)
        mArray[i] = getNumFromSBox(mArray[i]);
    return mergeArrayToInt(mArray);
}
/** 字节代换 */
void    SubBytes(int m[4]) {
    for (int i = 0; i < 4; i++)
        m[i] = SubWord(m[i]);
}
/** 行移位 */
void    ShiftRows(int m[4]) {
    int matrix[4][4] = { 0 };
    for (int i = 0; i < 4; i++)
        splitIntToArray(m[i], matrix[i]);
    for (int i = 0; i < 4; i++) { /** 操作第i行 */
        for (int j = 0; j < i; j++) { /** 共移位j次 */
            for (int k = 3; k > 0; k--) /** 移位一次 */
                matrix[0][i] ^= matrix[k][i] ^= matrix[0][i] ^= matrix[k][i];
        }
    }
    for (int i = 0; i < 4; i++) {
        m[i] = mergeArrayToInt(matrix[i]);
    }
}
/** 轮密钥加 */
void    AddRoundKey(int m[4], int round) {
    for (int i = 0; i < 4; i++) {
        m[i] = m[i] ^ w[round * 4 + i];
    }
}

/** 列混合
 * REFER: 关于AES的列混合计算和解密流程问题_aes列混合运算-CSDN博客
 * URL: https://blog.csdn.net/weixin_46395886/article/details/112793345
 */
/** 多项式模乘计算 */
int     GFMul(int n, int s) {
    int result = 0, a7 = 0;
    switch (n) {
    case 1:
        return s;
    case 2:
        result = s << 1;
        a7 = result & 0x00000100;
        if (a7 != 0) {
            result = result & 0x000000ff;
            result = result ^ 0x1b;
        }
        return result;
    case 3:
        return GFMul(2, s) ^ s;
    default:
        return 0;
    }
}
/** 列混合要用到的矩阵 */
const int colMatrix[4][4] = {
    2, 3, 1, 1,
    1, 2, 3, 1,
    1, 1, 2, 3,
    3, 1, 1, 2
};
/** 列混合 */
void    MixColumn(int m[4]) {
    for (int i = 0; i < 4; i++) {
        int mArray[4] = { 0 };
        int tArray[4] = { 0 };
        splitIntToArray(m[i], mArray);
        for (int j = 0; j < 4; j++) {
            tArray[j] = GFMul(colMatrix[j][0], mArray[0]) ^ GFMul(colMatrix[j][1], mArray[1])
                      ^ GFMul(colMatrix[j][2], mArray[2]) ^ GFMul(colMatrix[j][3], mArray[3]);
        }
        m[i] = mergeArrayToInt(tArray);
    }
}

/** 扩展密钥
 *  REFER: AES key schedule - Wikipedia
 *  URL: https://en.wikipedia.org/wiki/AES_key_schedule
 */
/** 常量轮值表 */
const int Rcon[10] = {
    0x01000000, 0x02000000,
    0x04000000, 0x08000000,
    0x10000000, 0x20000000,
    0x40000000, 0x80000000,
    0x1b000000, 0x36000000
};
/** 扩展密钥 */
int*    keyExpansion(char* k) {
    for (int i = 0; i < 60; i++) {
        if (i >= N && i % N == 0)
            w[i] = w[i - N] ^ SubWord(RotWord(w[i - 1])) ^ Rcon[(i / N) - 1];
        else if (i >= N && i % N == 4)
            w[i] = w[i - N] ^ SubWord(w[i - 1]);
        else if (i < N)
            w[i] = convertChrToInt(k + i * 4);
        else
            w[i] = w[i - N] ^ w[i - 1];
    }
    return w;
}

int main() {
    char key[33] = "zG2nSeEfSHfvTCHy5LCcqtBbQehKNLXn";
    /** 7a47326e 53654566 53486676 54434879 354c4363 71744262 5165684b 4e4c586e */
    const int pLen = 64;
    char plain[pLen + 1] = "CMhThvPhug72aYa8oJU6bydqGhLfqMjnb6oEwGGWcye98rxQag5pegAZVxsic5f5";
    /** 434d6854 68765068 75673732 61596138 6f4a5536 62796471 47684c66 714d6a6e 62366f45 77474757 63796539 38727851 61673570 6567415a 56787369 63356635 */
    char cipher[pLen] = { 0 };

    int* keyE = keyExpansion(key);
    int queueMatrix[4];

    for (int i = 0; i < pLen; i += 16) {
        splitChrTo4xMatrix(plain + i, queueMatrix);

        AddRoundKey(queueMatrix, 0);
        for (int j = 1; j < Nr; j++) { /** 前Nr-1轮 */
            SubBytes(queueMatrix);
            ShiftRows(queueMatrix);
            MixColumn(queueMatrix);
            AddRoundKey(queueMatrix, j);
        }
        /** 最后一轮 */
        SubBytes(queueMatrix);
        ShiftRows(queueMatrix);
        AddRoundKey(queueMatrix, Nr);

        merge4xMatrixToChr(queueMatrix, cipher + i);
    }

    std::cout << "= > B64EncodedCipher" << std::endl;
    std::cout << (new Base64)->Encode((unsigned char*)cipher, pLen) << std::endl;
}

void pass() {
}
