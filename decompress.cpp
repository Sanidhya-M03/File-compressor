#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <codecvt>
#include <locale>
#include <memory>
#include <bitset>

using namespace std;

struct HuffmanNode {
    wchar_t character;
    shared_ptr<HuffmanNode> left;
    shared_ptr<HuffmanNode> right;

    HuffmanNode(wchar_t ch) : character(ch), left(nullptr), right(nullptr) {}
    HuffmanNode(shared_ptr<HuffmanNode> left, shared_ptr<HuffmanNode> right) : character(L'\0'), left(left), right(right) {}
};

void buildHuffmanTree(shared_ptr<HuffmanNode>& root, const wstring& code, wchar_t character) {
    shared_ptr<HuffmanNode> currentNode = root;
    for (wchar_t bit : code) {
        if (bit == L'0') {
            if (!currentNode->left) currentNode->left = make_shared<HuffmanNode>(L'\0');
            currentNode = currentNode->left;
        } else {
            if (!currentNode->right) currentNode->right = make_shared<HuffmanNode>(L'\0');
            currentNode = currentNode->right;
        }
    }
    currentNode->character = character;
}

int main() {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    ifstream inputFile("output.bin", ios::binary);

    if (!inputFile.is_open()) {
        wcerr << L"Error opening file: output.bin" << endl;
        return 1;
    }

    // Read the size of the frequency table
    int mapSize;
    inputFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    // Read the Huffman codes from the file
    unordered_map<wchar_t, wstring> huffmanCode;
    for (int i = 0; i < mapSize; ++i) {
        wchar_t ch;
        int codeLength;
        inputFile.read(reinterpret_cast<char*>(&ch), sizeof(ch));
        inputFile.read(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));
        string narrowCode(codeLength, '\0');
        inputFile.read(&narrowCode[0], codeLength);
        huffmanCode[ch] = converter.from_bytes(narrowCode);
    }

    // Reconstruct the Huffman tree
    shared_ptr<HuffmanNode> root = make_shared<HuffmanNode>(L'\0');
    for (const auto& pair : huffmanCode) {
        buildHuffmanTree(root, pair.second, pair.first);
    }

    // Decode the compressed data
    ofstream outputFile("decompressed.txt", ios::binary);
    if (!outputFile.is_open()) {
        wcerr << L"Error opening output file: decompressed.txt" << endl;
        return 1;
    }

    bitset<8> bits;
    shared_ptr<HuffmanNode> currentNode = root;
    char byte;
    while (inputFile.read(reinterpret_cast<char*>(&byte), 1)) {
        bits = bitset<8>(byte);
        for (int i = 0; i < 8; ++i) {
            if (bits[i]) {
                currentNode = currentNode->right;
            } else {
                currentNode = currentNode->left;
            }

            // Found a leaf node
            if (!currentNode->left && !currentNode->right) {
                string outputChar = converter.to_bytes(currentNode->character);
                outputFile.write(outputChar.c_str(), outputChar.size());
                currentNode = root;
            }
        }
    }

    inputFile.close();
    outputFile.close();
    wcout << L"Decompressed data written to decompressed.txt" << endl;

    return 0;
}
