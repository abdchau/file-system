#include "../headers/FileSystem.h"



FileSystem::FileSystem(){
    initialize();
    Header root_header(0);
	current_dir = *(new Directory(0, "/", true, root_header, Entry("/", 0, true, true)));
    file_open = false;
    out_stream = &cout;
}

void FileSystem::initialize() {
    struct stat buffer;   
    
    if(!(stat (DATA_FILE, &buffer) == 0)) {
		ofstream my_file(DATA_FILE, ios::binary);
		char c = 0;
		for (int i = 0; i < ADDRESS_SPACE; i++)
			my_file.write(&c, 1);
		my_file.close();


        Header root_header(0);
        root_header.is_dir = root_header.is_occupied = true;
        root_header.write(0);
        root_header.read(root_header.block_no);
        Directory root(0, "/", true, root_header, Entry("/", 0, true, true));
		root.write();
	}
}


string FileSystem::mkdir(string file_name) {

    if ((int)file_name.length() > 30) 
    {
        return "File names cannot exceed 30 Characters!\n";
    }

    try {
        current_dir.find_entry(file_name);
        return "Folder already exists\n";
    }
    catch(int err) {
        char new_block = (char)find_empty_block(0);
        Header header(new_block, 0, 0, true, true);
        Directory dir(new_block, file_name, true, header, current_dir.entrify());
        dir.write();

        current_dir.add_entry(file_name, new_block, true, true);
    }
    return "";    
}

void FileSystem::open(string file_name) {
    if(file_open) {
        *out_stream << "A file is already open. Close it to open new file\n";
        return;
    }
    try {
        Entry entry = current_dir.find_entry(file_name);
        if (!entry.is_dir) {
            current_file = new File(entry);
            file_open = true;
        }
        else
            *out_stream << "File of name " << file_name << " not found\n";
    }
    catch (int err) {
        *out_stream << file_name << " not found\n";
    }
}

void FileSystem::close() {
    if(file_open) {
        delete current_file;
        file_open = false;
    }
}

void FileSystem::read(int start, int size) {
    if(file_open)
        *out_stream << current_file->read(start, size) << endl;
    else
        *out_stream << "No file opened\n";
}

void FileSystem::write(string file_contents, int start) {
    if(file_open) {
        try {
           current_file->write(file_contents, start);
        }
        catch(const char* err) {
            *out_stream << err << endl;
        }
    }
    else
        *out_stream << "No file opened\n";
}

void FileSystem::append(string file_contents) {
    if(file_open)
        current_file->write(file_contents, (int)current_file->read().length());
    else
        *out_stream << "No file opened\n";
}

void FileSystem::pwd() {
    *out_stream << current_dir.file_name << endl;
}

Directory FileSystem::cd(string dir_name, Directory dir){
    Entry entry;

    if (!dir_name.compare("..")) {
        return dir.parent_dir;
    }
    
    try {
        entry = dir.find_entry(dir_name, true, false);
        return Directory(entry);
    }
    catch(int err) {
        *out_stream << dir_name << " does not exist\n";
        throw(-1);
    }
    
}

string FileSystem::ls() {
    return current_dir.list_contents();
}

void FileSystem::rm(string file_name) {
    rm(file_name, false);
}

void FileSystem::rm(string file_name, bool recursive) {
    if (!file_name.compare("/")) {
        *out_stream << "ROOT DIRECTORY: DO NOT ATTEMPT TO DELETE OR DIRE THINGS WILL HAPPEN\n";
        return;
    }
    try {
        Entry entry = current_dir.find_entry(file_name);
        if (entry.is_dir) {
            Directory directory(entry);

            if (!directory.is_empty() && !recursive) {
                *out_stream << "Directory not empty. Use -r flag to delete recursively\n";
                return;
            }
            else
                directory.clear();
        }
        
        delete_file(entry);
    }
    catch(int err) {
        *out_stream << file_name << " not found\n";
        return;
    }
}

void FileSystem::mv(string source, string destination) {

    Directory source_dir;
    Directory dest_dir;

    vector<string> source_path = split_string(source, '/');
    vector<string> dest_path = split_string(destination, '/');

    if (source[0] == '/')
    {
        source_dir = Directory(Entry("/", 0, true, true));
        source_path[0] = "..";
    }
    else
    {
        source_dir = Directory(&current_dir);
    }
    

    if (destination[0] == '/')
    {
        dest_dir = Directory(Entry("/", 0, true, true));
        dest_path[0] = "..";
    }
    else
    {
        dest_dir = Directory(&current_dir);
    }
    
    

    for(int i = 0; i < (int)source_path.size()-1; i++) {
        try {
            source_dir = cd(source_path[i], source_dir);
        }
        catch(int i) {
            return;
        }
    }

    for(int i = 0; i < (int)dest_path.size()-1; i++) {
        try{
            dest_dir = cd(dest_path[i], dest_dir);
        }
        catch(int i) {
            return;
        }
    }

    Entry entry = source_dir.find_entry(source_path[source_path.size() - 1]);
    entry.clear();  entry.is_occupied = true;
    entry.file_name = dest_path[dest_path.size()-1];
    dest_dir.add_entry(entry);
}

void FileSystem::mkfile(string file_name) {
    try {
        current_dir.find_entry(file_name);
        *out_stream << "File already exists\n";
    }
    catch(int err) {
        File file(file_name);
        file.create();
        current_dir.add_entry(file_name, file.file_start, false, true);
    }
}

void FileSystem::stat_(string file_name) {

    Entry entry;
    try {
        entry = current_dir.find_entry(file_name);
        entry.print();
            entry.print();
        if (entry.is_dir) {
            Directory directory(entry);
            if (directory.is_empty())
                *out_stream << "Directory is empty\n";
            else
                *out_stream << "Directory is non-empty\n";
        }
        else
            *out_stream << "File exists\n";
    }
    catch(int err) {
        *out_stream << err;
        *out_stream << file_name << " not found\n";
        return;
    }
}

void FileSystem::view() {
    *out_stream << disk_usage();
    *out_stream << "\n\n";
    *out_stream << block_map();
}

void FileSystem::man() {
    *out_stream << get_manual();
}

void FileSystem::map(string file_name) {
    *out_stream << show_memory_map(current_dir.find_entry(file_name));
}

string FileSystem::run(string command) {
    vector<string> tokens = split_string(command, ' ');
    if (tokens.size() == 0)
        return "";

    
    if (!tokens[0].compare("ls") && tokens.size() < 2) {
        return ls();
    }
    else if (!tokens[0].compare("ls")  && !tokens[1].compare("-a")) {
        return current_dir.list_structure();
    }
    else if (!tokens[0].compare("mkdir")) {
        return mkdir(tokens[1]);
    }
    else if (tokens.size() > 2 && !tokens[0].compare("rm") && !tokens[1].compare("-r")) {
        if (tokens[2].size() == 0)
        {
            return "Invalid Command! for help type 'man'\n";
        }
        
        rm(tokens[2], true);
    }
    else if (!tokens[0].compare("rm") && tokens[1].compare("-r")) {
        if (tokens.size() < 2 || tokens[1].size() == 0)
        {
            return "Invalid Command! for help type 'man'\n";
        }
        rm(tokens[1]);
    }
    else if (!tokens[0].compare("cd")) {
        if (tokens[1].size() == 0)
            return "Invalid Command! for help type 'man'\n";
        
        try {
            Directory curr_dir = Directory(&current_dir);
            vector<string> path = split_string(tokens[1], '/');

            if (tokens[1][0] == '/') {
                while(curr_dir.parent_dir.file_name != "/")
                    curr_dir = curr_dir.parent_dir;
                path[0] = "..";
            }

            for(int i = 0; i < (int)path.size(); i++) {
                curr_dir = cd(path[i], curr_dir);
            }
            current_dir = curr_dir;
        }
        catch(int i) {
            return "";
        }
    }
    else if (!tokens[0].compare("pwd")) {
        pwd();
    }
    else if (!tokens[0].compare("stat")) {
        stat_(tokens[1]);
    }
    else if (!tokens[0].compare("map")) {
        if (tokens.size() < 2 || tokens[1].size() == 0)
            return "Invalid Command! for help type 'man'\n";
        map(tokens[1]);
    }
    else if (!tokens[0].compare("view")) {
        view();
    }
    else if (!tokens[0].compare("man")) {
        man();
    }
    else if (!tokens[0].compare("mkfile")) {
        mkfile(tokens[1]);
    }
    else if (!tokens[0].compare("open") && tokens.size() > 1) {
        open(tokens[1]);
    }
    else if (!tokens[0].compare("close")) {
        close();
    }
    else if (!tokens[0].compare("read")) {
        if (!file_open)
            return "No file open\n";

        int start, size;
        if (tokens.size() == 1) {
            start = 0;
            size = MAX_INT;
        }
        else if (tokens.size() == 2) {
            start = stoi(tokens[1]);
            size = MAX_INT;
        }
        else if (tokens.size() == 3) {
            start = stoi(tokens[1]);
            size = stoi(tokens[2]);
        }
        
        read(start, size);
    }
    else if (!tokens[0].compare("write")) {
        if (!file_open) {
            return "No file open\n";
        }
        string file_contents;
        
        int start;
        if (tokens.size() == 1) {
            start = 0;
            *out_stream << "Enter contents of file:\n";
            getline(cin, file_contents);
        }
        else if (tokens.size() == 2) {
            start = stoi(tokens[1]);
            *out_stream << "Enter contents of file:\n";
            getline(cin, file_contents);
        }
        else if (tokens.size() >= 3) {
            if (!tokens[1].compare("-s")) {
                file_contents = command.substr(tokens[0].size()+tokens[1].size()+2);
            }
            else if (!tokens[2].compare("-s")) {
                start = stoi(tokens[1]);
                file_contents = command.substr(tokens[0].size()+tokens[1].size()+tokens[2].size()+3);
            }
            else {
                return "Invalid Arguments\n";
            }
        }

        write(file_contents, start);
    }
    else if (!tokens[0].compare("append")) {
        if (!file_open) {
            return "No file open\n";
        }
        string file_contents;

        if (tokens.size() == 1) {
            return "Enter contents of file:\n";
            getline(cin, file_contents);
        }
        else if (tokens.size() >= 2) {
            if (!tokens[1].compare("-s")) {
                file_contents = command.substr(tokens[0].size()+tokens[1].size()+2);
            }
            else {
                return "Invalid Arguments\n";
            }
        }

        append(file_contents);
    }
    else if (!tokens[0].compare("trunc")) {
        if (!file_open) {
            return "No file open\n";
        }
        current_file->truncate(stoi(tokens[1]));
    }
    else if (!tokens[0].compare("mvwf")) {
        if (!file_open) {
            return "No file open\n";
        }
        current_file->move_within_file(stoi(tokens[1]), stoi(tokens[2]), stoi(tokens[3]));
    }
    else if (!tokens[0].compare("mv")) {
        if (tokens.size() < 2 || !tokens[1].size() || tokens[2].size())
        {
            return "Invalid Command! for help type 'man'\n";
        }
        
        mv(tokens[1], tokens[2]);
    }
    else
    {
        return "Invalid Command! for help type 'man'\n";
    }
    return "\n";
}

void FileSystem::run_script(ifstream& file) {
    while(file.peek() != EOF) {
		string command;
		getline(file, command);
		if (!command.compare("exit"))
			exit(-1);
		else {
			run(command);
		}
	}
}

void thread_wrapper(FileSystem file_system, string file_name) {
    struct stat buffer; 
    
    ifstream file(file_name);  
    ofstream out_file("out_" + file_name);
    file_system.out_stream = &out_file;
    if(!(stat (file_name.c_str(), &buffer) == 0)) {
        out_file << file_name << " does not exists\n";
    }
    else {    
        file_system.run_script(file);
    }
    file.close();
    out_file.close();
}