#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<cstdint>

enum record_type {
    RHEADR = 0x6e, //obs
    REGINT = 0x70, //obs
    REDATA = 0x72, //obs
    RIDATA = 0x74, //obs
    OVLDEF = 0x76, //obs
    ENDREC = 0x78, //obs
    BLKDEF = 0x7a, //obs
    BLKEND = 0x7c, //obs
    DEBSYM = 0x7e, //obs
    THEADR = 0x80,
    LHEADR = 0x82,
    PEDATA = 0x84, //obs
    PIDATA = 0x86, //obs
    COMENT = 0x88,
    MODEND = 0x8a,
    EXTDEF = 0x8c,
    TYPDEF = 0x8e, //obs
    PUBDEF = 0x90, //+1
    LOCSYM = 0x92, //obs
    LINNUM = 0x94, //+1
    LNAMES = 0x96,
    SEGDEF = 0x98, //+1
    GRPDEF = 0x9a,
    FIXUPP = 0x9c, //+1
    NONAME = 0x9e, //obs
    LEDATA = 0xa0, //+1
    LIDATA = 0xa2, //+1
    LIBHED = 0xa4, //obs
    LIBNAM = 0xa6, //obs
    LIBLOC = 0xa8, //obs
    LIBDIC = 0xaa, //obs
    COMDEF = 0xb0,
    BAKPAT = 0xb2, //+1
    LEXTDEF = 0xb4,
    LPUBDEF = 0xb6, //+1
    LCOMDEF = 0xb8,
    CEXTDEF = 0xbc,
    COMDAT = 0xc2, //+1
    LINSYM = 0xc4, //+1
    ALIAS = 0xc6,
    NBKPAT = 0xc8, //+1
    LLNAMES = 0xca,
    VERNUM = 0xcc,
    VENDEXT = 0xce,
    LIB_HEADER = 0xf0, //Start of library
    LIB_END            //End of library
};

void skip_padding(std::ifstream& in, uint16_t record_length) {
    size_t offset = in.tellg();
    int record_num = offset / record_length;
    if(offset % record_length > 0) {
        record_num++;
    }
    offset = record_num * record_length;
    in.seekg(offset);
}

void process_module(std::ifstream& in) {
    std::ofstream out;
    std::string module_name;
    std::cout<<"START MODULE\n";
    struct __attribute__((packed)) record_header {
        uint8_t record_type;
        uint16_t record_length;
    } h;
    h.record_type = LIB_HEADER;
    while(h.record_type != MODEND) {
        in.read(reinterpret_cast<char *>(&h), sizeof(record_header));
        std::vector<char> data(h.record_length);
        in.read(data.data(), h.record_length);
        std::cout<<std::hex<<int(h.record_type)<<" "<<int(h.record_length)<<" ";
        for(uint8_t d:data) {
            std::cout<<std::hex<<(unsigned int)(d)<<" ";
        }
        std::cout<<"\n";
        if(h.record_type == THEADR && !out.is_open()) {
            //module_name.resize(data[0]+1, '\0');
            module_name.insert(0,&(data.data()[1]),data[0]);
            module_name += ".OBJ";
            out.open(module_name);
            if(out.is_open()) {
                std::cout<<"Outputting module to "<<module_name<<"\n";
            }
            else {
                std::cerr<<"Couldn't open file "<<module_name<<"\n";
            }
        }
        out.write(reinterpret_cast<char *>(&h), sizeof(record_header));
        out.write(reinterpret_cast<char *>(data.data()), data.size());
        if(h.record_type == MODEND) {
            out.close();
        }
    }
    std::cout<<"END MODULE\n";
}

void process_library(std::ifstream& in) {
    struct __attribute__((packed)) lib_header {
        uint8_t record_type;
        uint16_t record_length;
        uint32_t dict_offset;
        uint16_t dict_size; //in blocks
        uint8_t flags;
    } h;
    //static_assert(sizeof(lib_header) == 10);
    in.read(reinterpret_cast<char *>(&h), sizeof(lib_header));
    if(h.record_type != LIB_HEADER) {
        std::cerr<<"File doesn't look like an OMF library. Exiting.\n";
        return;
    }
    h.record_length += 3;
    std::cout<<"Record type: "<<std::hex<<int(h.record_type)<<"\nRecord length: "<<h.record_length<<"\nDictionary offset: "<<h.dict_offset<<"\nDictionary size: "<<h.dict_size<<" (blocks)\nFlags: "<<int(h.flags)<<"\n";

    record_type current_record = LIB_HEADER;
    while(current_record != LIB_END) {
        skip_padding(in, h.record_length);
        current_record = (record_type)in.peek();
        if(current_record != LIB_END) {
            process_module(in);
        }
    }

    std::cout<<"LIBEND\n";

    return;
}

int main(int argc, char *argv[]) {
    std::string in_filename;
    if(argc == 2) {
        in_filename = argv[1];
    }
    else {
        std::cerr<<"Use: "<<argv[0]<<" infile\n";
        return 1;
    }

    std::ifstream in_filestream(in_filename);

    if(!in_filestream.is_open()) {
        std::cerr<<"Couldn't open input file\n";
        return 1;
    }

    process_library(in_filestream);

    in_filestream.close();

    return 0;
}
