#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <archive.h>
#include <archive_entry.h>
#include <json/json.h>

typedef std::pair<std::shared_ptr<uint8_t>, size_t> safe_array_t;

struct docker_image_manifest
{
    std::string config;
    std::vector<std::string> repoTags;
    std::vector<std::string> layer;
};

docker_image_manifest parseManifestFile(safe_array_t data)
{
    docker_image_manifest dim;
    Json::Value root;
    std::stringstream ss;
    ss << data.first;
    ss >> root;
    auto manifest = root.get(Json::Value::ArrayIndex(0), Json::Value());

    std::string config = manifest.get("Config", "").asString();
    dim.config = config;
    std::cerr << "Config: " << config << std::endl;
    auto repoTags = manifest.get("RepoTags", Json::Value());
    for (size_t i = 0; i < repoTags.size(); i++)
    {
        std::string repoTag = repoTags.get(Json::Value::ArrayIndex(i), Json::Value()).asString();
        dim.repoTags.push_back(repoTag);
        std::cerr << "RepoTag: " << repoTag << std::endl;
    }
    auto layers = manifest.get("Layers", Json::Value());
    for (size_t i = 0; i < layers.size(); i++)
    {
        std::string layer = layers.get(Json::Value::ArrayIndex(i), Json::Value()).asString();
        dim.layer.push_back(layer);
        std::cerr << "Layer: " << layer << std::endl;
    }

    return dim;
}

std::vector<std::string> parseConfigJsonFile(safe_array_t data)
{
    std::vector<std::string> r;
    docker_image_manifest dim;
    Json::Value root;
    std::stringstream ss;
    ss << data.first;
    ss >> root;
    auto cc = root.get("history", Json::Value());
    for (size_t i = 0; i < cc.size(); i++)
    {
        auto obj = cc.get(i, Json::Value());
        std::string cmd = obj.get("created_by", "").asString();
        bool empty = obj.get("empty_layer", false).asBool();
        if (!empty)
            r.push_back(cmd);
        std::cerr << "Cmd: " << cmd << std::endl;
    }
    return r;
}

void parseLayerJsonFile(safe_array_t data)
{
    docker_image_manifest dim;
    Json::Value root;
    std::stringstream ss;
    ss << data.first;
    ss >> root;
    auto cc = root.get("container_config", Json::Value());
    auto cmda = cc.get("Cmd", Json::Value());
    std::string cmd;
    for (size_t i = 0; i < cmda.size(); i++)
    {
        if (i > 0)
            cmd += " ";
        cmd += cmda.get(i, "").asString();
    }
    std::cerr << "Cmd: " << cmd << std::endl;
}

safe_array_t readOneFile(const std::string& arcFile, const std::string& file)
{
    safe_array_t result;
    auto archive = archive_read_new();
    archive_read_support_filter_all(archive);
    archive_read_support_format_all(archive);
    int r = archive_read_open_filename(archive, arcFile.c_str(), 10240); // Note 1
    if (r != ARCHIVE_OK)
    {
        std::cerr << "archive_read_open_filename failed" << std::endl;
        return safe_array_t();
    }

    struct archive_entry* entry;
    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK)
    {
        std::string fileName = archive_entry_pathname(entry);
        if (fileName == file)
        {
            size_t fileSize = archive_entry_size(entry);
            std::shared_ptr<uint8_t> fileBuf(new uint8_t[fileSize + 1]);
            auto readSize = archive_read_data(archive, fileBuf.get(), fileSize);
            if (readSize < 0)
            {
                std::cerr << "archive_read_data failed" << std::endl;
                continue;
            }
            fileBuf.get()[fileSize] = 0;
            result = std::make_pair(fileBuf, fileSize);
            break;
        }
        //archive_read_data_skip(archive);  // Note 2
    }
    r = archive_read_free(archive);  // Note 3
    if (r != ARCHIVE_OK)
    {
        std::cerr << "archive_read_free failed" << std::endl;
        return safe_array_t();
    }
    return result;
}

safe_array_t readOneFile(const safe_array_t &input, const std::string& file)
{
    safe_array_t result;
    auto archive = archive_read_new();
    archive_read_support_filter_all(archive);
    archive_read_support_format_all(archive);
    int r = archive_read_open_memory(archive, input.first.get(), input.second);
    if (r != ARCHIVE_OK)
    {
        std::cerr << "archive_read_open_filename failed" << std::endl;
        return safe_array_t();
    }

    struct archive_entry* entry;
    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK)
    {
        std::string fileName = archive_entry_pathname(entry);
        if (fileName == file)
        {
            size_t fileSize = archive_entry_size(entry);
            std::shared_ptr<uint8_t> fileBuf(new uint8_t[fileSize + 1]);
            auto readSize = archive_read_data(archive, fileBuf.get(), fileSize);
            if (readSize < 0)
            {
                std::cerr << "archive_read_data failed" << std::endl;
                continue;
            }
            fileBuf.get()[fileSize = 0];
            result = std::make_pair(fileBuf, fileSize);
            break;
        }
        //archive_read_data_skip(archive);  // Note 2
    }
    r = archive_read_free(archive);  // Note 3
    if (r != ARCHIVE_OK)
    {
        std::cerr << "archive_read_free failed" << std::endl;
        return safe_array_t();
    }
    return result;
}

std::map<std::string, size_t> listFiles(const safe_array_t& input)
{
    std::map<std::string, size_t> result;
    auto archive = archive_read_new();
    archive_read_support_filter_all(archive);
    archive_read_support_format_all(archive);
    int r = archive_read_open_memory(archive, input.first.get(), input.second);
    if (r != ARCHIVE_OK)
    {
        std::cerr << "archive_read_open_filename failed" << std::endl;
        return result;
    }

    struct archive_entry* entry;
    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK)
    {
        std::string fileName = archive_entry_pathname(entry);
        result[fileName] = archive_entry_size(entry);
        //archive_read_data_skip(archive);  // Note 2
    }
    r = archive_read_free(archive);  // Note 3
    if (r != ARCHIVE_OK)
    {
        std::cerr << "archive_read_free failed" << std::endl;
        return result;
    }
    return result;
}

static bool isWhiteoutOpaque(const std::string& s)
{
    size_t offset = s.rfind('/') + 1;
    std::string t = s.substr(offset);
    return t == ".wh..wh..opq";
}

static std::string getWhiteout(const std::string& s)
{
    size_t offset = s.rfind('/') + 1;
    std::string t = s.substr(offset);
    if ((t.length() > 4) && (t.substr(0, 4) == ".wh."))
        return t.substr(4);
    return "";
}

static void removeFile(std::map<std::string, size_t>& map, const std::string& file)
{
    for (auto it = map.begin(); it != map.end(); )
    {
        if (it->first == file)
        {
            std::cout << "  Delete      " << it->first << std::endl;
            it = map.erase(it);
        }
        else
            ++it;
    }
}

static void removeDir(std::map<std::string, size_t> &map, const std::string &file)
{
    for (auto it = map.begin(); it != map.end(); )
    {
        if ((it->first.length() >= file.length()) && (it->first[file.length()] == '/') && (it->first.substr(0, file.length()) == file))
        {
            std::cout << "  Delete      " << it->first << std::endl;
            it = map.erase(it);
        }
        else
            ++it;
    }
}

static std::string formatFileSize(uint64_t n)
{
    uint64_t cmp = 1;
    if (n < cmp * 1024)
        return std::to_string(n / cmp) + " ";
    cmp *= 1024;
    if (n < cmp * 1024)
        return std::to_string(n / cmp) + "K";
    cmp *= 1024;
    if (n < cmp * 1024)
        return std::to_string(n / cmp) + "M";
    cmp *= 1024;
    if (n < cmp * 1024)
        return std::to_string(n / cmp) + "G";
    cmp *= 1024;
    return std::to_string(n / cmp) + "T";
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "docker-image-viewer" << std::endl;
        std::cerr << "usage: tar_file" << std::endl;
        return 1;
    }

    std::cerr << "docker-image-viewer" << std::endl;
    std::string arcFile = argv[1];
    auto manifest = readOneFile(arcFile, "manifest.json");
    if (manifest.first == nullptr)
    {
        std::cerr << "readOneFile manifest.json failed" << std::endl;
        return 2;
    }

    auto dim = parseManifestFile(manifest);
    auto config = readOneFile(arcFile, dim.config);
    if (config.first == nullptr)
    {
        std::cerr << "readOneFile " << dim.config << " failed" << std::endl;
        return 2;
    }

    auto dic = parseConfigJsonFile(config);
    std::map<std::string, size_t> fileMap;
    int i = 0;
    for (auto tarFileName : dim.layer)
    {
        //auto jsonFileName = tarFileName.substr(0, tarFileName.rfind('/') + 1) + "json";
        //auto jsonFile = readOneFile(arcFile, jsonFileName);
        //parseLayerJsonFile(jsonFile);
        auto tarFile = readOneFile(arcFile, tarFileName);
        std::cout << tarFileName << std::endl;
        std::cout << dic[i++] << std::endl;
        auto layerFileList = listFiles(tarFile);
        for (auto lfn : layerFileList)
        {
            if (isWhiteoutOpaque(lfn.first)) continue;
            auto wh = getWhiteout(lfn.first);
            if (!wh.empty())
            {
                removeFile(fileMap, wh);
                removeDir(fileMap, wh);
            }
            else
            {
                removeDir(fileMap, lfn.first);
                fileMap[lfn.first] = lfn.second;
                std::cout << "  Write " << std::setw(5) << formatFileSize(lfn.second) << " " << lfn.first << std::endl;
            }
        }
    }

	return 0;
}
