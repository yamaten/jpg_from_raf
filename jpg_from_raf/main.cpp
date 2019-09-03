//
//  main.cpp
//  jpg_from_raf
//
//  Created by YAMAMOTO TENSHI on 2019/08/12.
//  Copyright © 2019 YAMAMOTO TENSHI. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <stdlib.h>

using namespace std;

struct RAF_HEADER {
    char MAGIC_STR[20];    // 'FUJIFILMCCD-RAW_0201'
    char MODEL_ID[8];
    char MODEL_NAME[32];
    char FARM_VER[4];
    char RESV1[20];
    unsigned JPEG_OFFSET;
    unsigned JPEG_SIZE;
    unsigned CFA_HEAD_OFFSET;
    unsigned CFA_HEAD_SIZE;
    unsigned CFA_BODY_OFFSET;
    unsigned CFA_BODY_SIZE;
};

unsigned _cnv_endian(unsigned x) {
//    return ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8)
//        | ((x & 0x00ff0000) >> 8) | ((x & 0xff000000) >> 24);

    return ((x <<24) & 0xff000000)  | ((x << 8) & 0x00ff0000)
        | ((x >> 8) & 0x0000ff00)  | ((x >> 24) & 0x000000ff);
}

size_t _to_size(unsigned x) {
    return (size_t)_cnv_endian(x);
}

unsigned _to_long(unsigned x) {
    return _cnv_endian(x);
}


static void dir_err(errno_t err)
{
    switch (err) {
        case EACCES: cerr << "アクセス権限がない。" << endl; break;
        case EBADF: cerr << "fd が読み出し用にオープンされた、有効なファイルディスクリプターではない。" << endl; break;
        case EMFILE: cerr << "プロセスが使用中のファイルディスクリプターが多すぎる。" << endl; break;
        case ENFILE: cerr << "システムでオープンされているファイルが多すぎる。" << endl; break;
        case ENOENT: cerr << "ディレクトリが存在しないか、または name が空文字列である。" << endl; break;
        case ENOMEM: cerr << "操作を完了するのに十分なメモリーがない。" << endl; break;
        case ENOTDIR: cerr << "name はディレクトリではない。" << endl; break;
    }
    
    exit(1);
}


static vector<string> dir_read(char *folder, char *filter=NULL)
{
    cout << folder;
    size_t filter_len = 0;
    if (filter != NULL) {
        filter_len = strlen(filter);
        cout << " | " << filter;
    }
    cout << endl;
    
    vector<string> fileList;
    DIR *dir = opendir(folder);
    if (dir == NULL) {
        dir_err(errno);
    }
    else {
        struct dirent *dp = readdir(dir);
        while (dp != NULL) {
            // cerr << dp->d_name << endl;
            string str = string(dp->d_name);
            if (filter_len < str.length()
             && str.substr(str.length()-filter_len).compare(filter) == 0) {
                fileList.push_back(str);
            }
            dp = readdir(dir);
        }
        
        if (dir != NULL)
            closedir(dir);
    }
    return fileList;
}

int jpg_from_raf(string src, string dst)
{
    // フォルダ内の画像名の取得
    char filter[] = ".RAF";
    vector<std::string> file_names = dir_read((char*)src.c_str(), filter);
    
    int out_cnt = 0;
    for (string fname : file_names) {
        string full_path = src + "/" + fname;
        
        FILE* raf_in = fopen(full_path.c_str(), "rb");
        if (raf_in == NULL) {
            cout << "file:" << full_path << " can't open." << endl;
            exit(1);
        }
        
        RAF_HEADER raf_header;
        size_t cnt = fread(&raf_header, sizeof(raf_header), 1, raf_in);
        if (cnt > 0) {
            string magic_str  = string(raf_header.MAGIC_STR, sizeof(raf_header.MAGIC_STR));
            string model_id   = string(raf_header.MODEL_ID, sizeof(raf_header.MODEL_ID));
            string magic_name = string(raf_header.MODEL_NAME, sizeof(raf_header.MODEL_NAME));
            string farm_ver   = string(raf_header.FARM_VER, sizeof(raf_header.FARM_VER));
            /*
             cout << magic_str  << endl;
             cout << model_id   << endl;
             cout << magic_name << endl;
             cout << farm_ver   << endl;
             */
            
            string jpg_file = dst + "/" + fname + "_000.jpg";
            cout << "ファイル '" << jpg_file <<"' "; // "' 抽出" << endl;
            unsigned  jpg_offset  = _to_long(raf_header.JPEG_OFFSET);
            size_t jpg_size = _to_size(raf_header.JPEG_SIZE);
            char* jpg_buff = new char[jpg_size];
            fseek(raf_in, jpg_offset, SEEK_SET);
            cnt = fread(jpg_buff, jpg_size, 1, raf_in);
            
            FILE* jpg_o = fopen(jpg_file.c_str(), "wb+");
            fwrite(jpg_buff, jpg_size, 1, jpg_o);
            fclose(jpg_o);
            out_cnt++;
            std::cout << jpg_size << " bytes 完了\n";
            delete [] jpg_buff;
        }
        fclose(raf_in);
    }
    return out_cnt;
}


int main(int argc, const char * argv[]) {
    // insert code here...
    //std::cout << "Hello, World!\n";
    //return 0;
    string fpath = ".";
    string dpath = ".";
    //fpath = "/Users/tenshi/Pictures/Capture One/2019-08-17.X-E3";
    //dpath = fpath;
    cout << "RAF ファイルから JPEG を抽出\n";
    for (int i=0; i<argc; i++)
        cout << "argv[" << i << "]" << argv[i] << endl;

    
    switch (argc) {
    default:
        cout << "Usage: jpg_from_raf <src-dir> [<dst-dir>]" << endl;
        exit(1);
        
    case 2:
        fpath = argv[1];
        break;
        
    case 3:
        fpath = argv[1];
        dpath = argv[2];
        break;
    }

    jpg_from_raf(fpath, dpath);
    
    return 0;
}

