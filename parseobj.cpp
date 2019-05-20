#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<cstdint>
#include<cassert>

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
    NEW_OMF = 0xa1,
    LINK_PASS = 0xa2,
    DEPENDENCY_FILE = 0xe9,
};

struct segdef {
    uint8_t attributes;
    uint16_t length;
    uint8_t name_index;
    uint8_t class_name_index;
    uint8_t overlay_name_index;
    uint8_t checksum;
};

struct grpdef {
    uint8_t name_index;
    std::vector<uint8_t> seg_index;
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

    std::vector<std::string> namelist(0);
    std::vector<segdef> seglist(0);
    std::vector<grpdef> grplist(0);

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
                            std::cout<<"COMENT TRANSLATOR: "<<comment_string.substr(1)<<"\n";
                            break;
                        case NEW_OMF:
                            std::cout<<"COMENT NEW OMF EXTENSION\n";
                            break;
                        case LINK_PASS:
                            std::cout<<"COMENT LINK PASS 2 (data:"; 
                            for(unsigned char d:comment_string) {
                                std::cout<<std::hex<<int(d)<<" ";
                            }
                            std::cout<<")\n";
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
                            std::cout<<"COMENT length: "<<std::dec<<comment_length<<" type: "<<std::hex<<int(comment_type)<<" class: "<<int(comment_class)<<" data: \n\t";
                            for(unsigned char d:comment_string) {
                                if(d >= 0x20 && d < 0x7f) {
                                    std::cout<<"  "<<d;
                                }
                                else {
                                    std::cout<<"  .";
                                }
                            }
                            std::cout<<"\n\t";
                            for(unsigned char d:comment_string) {
                                std::printf(" %02X", d);
                                //std::cout<<std::hex<<" "<<int(d);
                            }
                            std::cout<<"\n";
                    }

                }
                break;
            case MODEND:
                std::cout<<"MODEND (TODO)\n";
                break;
            case PUBDEF:
                std::cout<<"PUBDEF (TODO)\n";
                break;
            case LINNUM:
                std::cout<<"LINNUM (TODO)\n";
                break;
            case LNAMES: {
                    size_t start_offset = 0;
                    size_t remaining = h.record_length - 1;
                    std::cout<<"LNAMES length: "<<h.record_length<<std::endl;
                    while(remaining > 0) {
                        size_t str_len = data[start_offset];
                        std::string str(&data[start_offset+1], str_len);
                        namelist.push_back(str);
                        std::cout<<"\t"<<namelist.size()<<": "<<str<<"\n";
                        remaining -= (str_len + 1);
                        start_offset += (str_len + 1);
                    }
                }
                break;
            case SEGDEF:
                std::cout<<"SEGDEF attributes: "<<std::hex<<int(data[0])<<" length: "<<(uint16_t(data[2])*256 + uint16_t(data[1]))<<" seg name: "<<namelist[data[3]-1]<<" class name: "<<namelist[data[4]-1]<<" overlay name: "<<namelist[data[5]-1]<<"\n";
                seglist.emplace_back(segdef{uint8_t(data[0]),uint16_t(uint16_t(data[2])*256+uint16_t(data[1])),uint8_t(data[3]),uint8_t(data[4]),uint8_t(data[5]),uint8_t(data[6])});
                break;
            case GRPDEF:
                {
                    std::cout<<"GRPDEF name: "<<namelist[data[0]-1]<<" segments: ";
                    int index = 1;
                    grplist.emplace_back(grpdef{uint8_t(data[0]),std::vector<uint8_t>(0)});
                    while(data[index] == -1) {
                        std::cout<<std::hex<<int(data[index+1])<<"("<<namelist[seglist[data[index+1]-1].name_index - 1]<<") ";
                        grplist.back().seg_index.push_back(uint8_t(data[index+1]));
                        index+=2;
                    }
                    std::cout<<"\n";
                }
                break;
            case FIXUPP:
                {
                    std::cout<<"FIXUPP "<<std::hex<<int(h.record_type)<<" (unknown) length: "<<std::dec<<int(h.record_length)<<" data:\n";
                    int index = 0;
                    while(index < data.size()) {
                        if((uint8_t(data[index]) & 0x80) == 0x80) { //FIXUP subrecord
                            std::cout<<"\tFIXUP";
                            if((uint8_t(data[index]) & 0x40) == 0x40) {
                                std::cout<<" (rel to seg)";
                            }
                            else {
                                std::cout<<" (rel to self)";
                            }
                            switch(uint8_t(data[index])/4 & 0x0f) {
                            case 0:
                                std::cout<<" low-order byte";
                                break;
                            case 1:
                                std::cout<<" 16-bit offset";
                                break;
                            case 2:
                                std::cout<<" 16-bit base";
                                break;
                            case 3:
                                std::cout<<" 32-bit pointer";
                                break;
                            case 4:
                                std::cout<<" high-order byte";
                                break;
                            case 5:
                                std::cout<<" 16-bit loader-resolved";
                                break;
                            case 6:
                                std::cout<<" reserved (6)";
                                break;
                            case 7:
                                std::cout<<" reserved (7)";
                                break;
                            case 8:
                                std::cout<<" reserved (8)";
                                break;
                            case 9:
                                std::cout<<" 32-bit offset";
                                break;
                            case 10:
                                std::cout<<" reserved (10)";
                                break;
                            case 11:
                                std::cout<<" 48-bit pointer";
                                break;
                            case 12:
                                std::cout<<" reserved (12)";
                                break;
                            case 13:
                                std::cout<<" 32-bit loader-resolved";
                                break;
                            default:
                                std::cout<<" derp. Skipping rest of entries.\n";
                                index = data.size();
                            }
                            uint16_t location = (uint16_t(data[0]) & 0x03) * 256 + uint16_t(data[1]);
                            std::cout<<" location: "<<std::hex<<location;
                            std::cout<<" Yep, still working on it.\n";
                            index = data.size();
                        }
                        else { //THREAD subrecord
                            std::cout<<"\tTHREAD";
                            if((uint8_t(data[index]) & 0x40) == 0x40) { //FRAME thread
                                std::cout<<" (FRAME)";
                            }
                            else { //TARGET thread
                                std::cout<<" (TARGET)";
                            }
                            std::cout<<" (haven't figured out size of the record, so I'm skipping the rest of the thing)\n";
                            index = data.size();
                        }
                    }
                    for(unsigned char d:data) {
                        std::cout<<std::hex<<int(d)<<" ";
                    }
                    std::cout<<"\n";
                }
                break;
            case LEDATA:
                std::cout<<"LEDATA length: "<<data.size() - 4<<" segment: "<<(unsigned int)(data[0])<<"("<<namelist[seglist[data[0] - 1].name_index - 1]<<") at offset: "<<((uint16_t(data[2]) * 256) + (uint16_t(data[1])))<<"\n";
                for(int i=3;i<data.size()-1;i++) {
                    if(((i-3) % 32) == 0) std::cout<<std::hex<<i-3<<"\t";
                    std::printf("%02x ", uint8_t(data[i]));
                    if(((i-2)%32) == 0) std::cout<<"\n";
                }
                std::cout<<"\n";
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
