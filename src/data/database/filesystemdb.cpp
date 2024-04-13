#include "filesystemdb.h"

using namespace cerberus::db;

//=============================================================================
FilesystemDB::FilesystemDB() {}
//=============================================================================
cerberus::OpRes FilesystemDB::init(const string &parameters) {}
//=============================================================================
void FilesystemDB::deinit() {}
//=============================================================================
cerberus::OpRes FilesystemDB::command(const string &query) {}
//=============================================================================
cerberus::OpResData<cerberus::DBTableBlock> FilesystemDB::queryBlock(const string &query) {}
//=============================================================================
cerberus::OpResData<cerberus::DBTableProto> FilesystemDB::queryPrototype(const string &tableName) {}
//=============================================================================
FilesystemDB::~FilesystemDB() {}
//=============================================================================
