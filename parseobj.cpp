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

enum comment_class {
    TRANSLATOR = 0,
    DEPENDENCY_FILE = 0xe9,
};

void process_module(std::ifstream& in) {
    std::cout<<"START MODULE\n";
    struct __attribute__((packed)) record_header {
        uint8_t record_type;
        uint16_t record_length;
    } h;
    h.record_type = in.peek();
    if(h.record_type != THEADR) {
        std::cerr<<"File doesn't look like an OMF object\n";
        return;
    }
    while(h.record_type != MODEND) {
        in.read(reinterpret_cast<char *>(&h), sizeof(record_header));
        std::vector<char> data(h.record_length);
        in.read(data.data(), h.record_length);
        switch(h.record_type) {
            case THEADR: {
                    std::string module_name(&(data[1]), data[0]);
                    std::cout<<"THEADR Name: "<<module_name<<"\n";
                }
                break;
            case COMENT: {
                    size_t comment_length = h.record_length - 3;
                    uint8_t comment_type = data[0];
                    uint8_t comment_class = data[1];
                    std::string comment_string(&(data[2]), comment_length);
                    switch(comment_class) {
                        case TRANSLATOR:
                            std::cout<<"COMENT TRANSLATOR: "<<comment_string<<"\n";
                            break;
                            
                        case DEPENDENCY_FILE: {
                                if(!comment_length) {
                                    std::cout<<"COMENT DEPENDENCY, length 0\n";
                                    continue;
                                }
                                uint32_t dos_timestamp = *reinterpret_cast<const uint32_t *>(comment_string.data());
                                size_t filename_length = (size_t)comment_string[4];
                                comment_string = comment_string.substr(5, filename_length);
                                std::cout<<"COMENT DEPENDENCY: timestamp: "<<std::hex<<dos_timestamp<<" filename : "<<comment_string<<" (length: "<<std::dec<<filename_length<<")\n";
                            }
                            break;
                            
                        default:
                            std::cout<<"COMENT length: "<<std::dec<<comment_length<<" type: "<<std::hex<<int(comment_type)<<" class: "<<int(comment_class)<<" data: ";
                            for(unsigned char d:comment_string) {
                                std::cout<<std::hex<<int(d)<<" ";
                            }
                            std::cout<<"\n";
                    }

                }
                break;
            default:
                std::cout<<std::hex<<int(h.record_type)<<" (unknown) length: "<<std::dec<<int(h.record_length)<<" data: ";
                for(unsigned char d:data) {
                    std::cout<<std::hex<<int(d)<<" ";
                }
                std::cout<<"\n";
        }
    }
    std::cout<<"END MODULE\n";
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

    process_module(in_filestream);

    in_filestream.close();

    return 0;
}