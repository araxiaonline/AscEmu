/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DatabaseUpdater.hpp"
#include <Logging/Logger.hpp>
#include "Database.h"
#include "Field.hpp"
#include <Common.hpp>
#include <Utilities/Util.hpp>
#include <iostream>

void DatabaseUpdater::initBaseIfNeeded(const std::string& dbName, const std::string& dbBaseType, Database& dbPointer)
{
    QueryResult* dbResult = dbPointer.Query("SHOW TABLES FROM %s", dbName.c_str());
    if (dbResult == nullptr)
    {
        sLogger.info("Database: Your Database %s has no tables. AE is setting up the database for you.", dbName.c_str());
        setupDatabase(dbBaseType, dbPointer);
    }

    // save 100% (current queue)
    if (auto const queue = dbPointer.GetQueueSize())
    {
        // set up bar size
        const int barSize = 70;

        // set up progress
        float currentProgress = 0.0f;

        // update the line
        while (currentProgress < 1.0f)
        {
            // calc percentage
            const uint32_t sendQueues = queue - dbPointer.GetQueueSize();
            currentProgress = static_cast<float>(sendQueues) / static_cast<float>(queue);

            std::cout << "Creating '" << dbName << "' : ";
            const auto position = static_cast<int>(barSize * currentProgress);
            for (auto i = 0; i < barSize; ++i)
            {
                if (i < position)
                    std::cout << "-";
                else if (i == position)
                    std::cout << ">";
                else
                    std::cout << " ";
            }

            std::cout << " | " << int(currentProgress * 100.0f);
            if (currentProgress < 1.0f)
            {
                std::cout << " %\r";
                std::cout.flush();
            }
            else
                std::cout << " %\n";

            Arcemu::Sleep(250);
        }
    }
}

void DatabaseUpdater::setupDatabase(const std::string& database, Database& dbPointer)
{
    const std::string sqlBaseDir = "sql/" + database;
    fs::path baseFilePath = fs::current_path();
    baseFilePath /= sqlBaseDir + "/" + database + "_base.sql";

    if (fs::exists(baseFilePath))
    {
        sLogger.debug("%s", baseFilePath.c_str());
        std::string loadedFile = Util::readFileIntoString(baseFilePath);

        // split into seperated string
        std::vector<std::string> seglist;
        std::string delimiter = ";\n";

        size_t pos = 0;
        std::string token;
        while ((pos = loadedFile.find(delimiter)) != std::string::npos)
        {
            token = loadedFile.substr(0, pos);
            seglist.emplace_back(token + ";");
            loadedFile.erase(0, pos + delimiter.length());
        }

        for (const auto& statements : seglist)
            dbPointer.ExecuteNA(statements.c_str());
    }
}

void DatabaseUpdater::checkAndApplyDBUpdatesIfNeeded(const std::string& database, Database& dbPointer)
{
    applyUpdatesForDatabase(database, dbPointer);

    while (dbPointer.GetQueueSize() > 0)
    {
        sLogger.info("-- busy updating database \"%s\". Waiting for %u queries to be executed.", database.c_str(), dbPointer.GetQueueSize());
        Arcemu::Sleep(500);
    }
}

struct DatabaseUpdateFile
{
    std::string fullName;
    uint32_t majorVersion;
    uint32_t minorVersion;
};

void DatabaseUpdater::applyUpdatesForDatabase(const std::string& database, Database& dbPointer)
{
    const std::string sqlUpdateDir = "sql/" + database + "/updates";

    //////////////////////////////////////////////////////////////////////////////////////////
    // 1. get current version
    QueryResult* result = dbPointer.Query("SELECT LastUpdate FROM %s_db_version ORDER BY LastUpdate DESC LIMIT 1", database.c_str());

    if (!result)
    {
        sLogger.failure("%s_db_version query failed!", database.c_str());
        return;
    }

    Field* fields = result->Fetch();
    const std::string dbLastUpdate = fields[0].GetString();

    sLogger.info("Database %s Version : %s", database.c_str(), dbLastUpdate.c_str());

    const auto lastUpdateMajor = Util::readMajorVersionFromString(dbLastUpdate);
    const auto lastUpdateMinor = Util::readMinorVersionFromString(dbLastUpdate);

    //////////////////////////////////////////////////////////////////////////////////////////
    // 2. check if update folder exist in *dir*/sql/
    std::map<uint32_t, DatabaseUpdateFile> updateSqlStore;

    uint32_t count = 0;
    std::vector<std::string> updateFiles;

    for (auto& p : fs::recursive_directory_iterator(sqlUpdateDir))
    {
        const std::string filePathName = p.path().string();
        updateFiles.push_back(filePathName);
    }

    // In Windows, recursive_directory_iterator seems to get files sorted but
    // in Linux they are in random order -Appled
    std::sort(updateFiles.begin(), updateFiles.end());

    for (const auto& filePathName : updateFiles)
    {
        std::string fileName = filePathName;
        fileName.erase(0, sqlUpdateDir.size() + 1);

        const uint32_t majorVersion = Util::readMajorVersionFromString(fileName);
        const uint32_t minorVersion = Util::readMinorVersionFromString(fileName);

        DatabaseUpdateFile dbUpdateFile;
        dbUpdateFile.fullName = filePathName;
        dbUpdateFile.majorVersion = majorVersion;
        dbUpdateFile.minorVersion = minorVersion;

        //\todo Remove me
        //sLogger.info("Available file in updates dir: %s", filePathName.c_str());

        updateSqlStore.emplace(std::pair<uint32_t, DatabaseUpdateFile>(count, dbUpdateFile));
        ++count;
    }

    updateFiles.clear();

    //////////////////////////////////////////////////////////////////////////////////////////
    // 3. save filenames into vector, when newer than current db version
    std::map<uint32_t, DatabaseUpdateFile> applyNewUpdateFilesStore;

    if (!updateSqlStore.empty())
    {
        //sLogger.debug("=========== New %s update files in %s ===========", database.c_str(), sqlUpdateDir.c_str());
        //compare it with latest update in mysql
        for (const auto update : updateSqlStore)
        {
            bool addToUpdateFiles = false;
            if (update.second.majorVersion == lastUpdateMajor && update.second.minorVersion > lastUpdateMinor)
                addToUpdateFiles = true;

            if (update.second.majorVersion > lastUpdateMajor)
                addToUpdateFiles = true;

            if (addToUpdateFiles)
            {
                applyNewUpdateFilesStore.insert(update);
                sLogger.debug("Updatefile %s, Major(%u), Minor(%u) - added and ready to be applied!", update.second.fullName.c_str(), update.second.majorVersion, update.second.minorVersion);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // 4. open/parse files and apply to db
    if (!applyNewUpdateFilesStore.empty())
    {
        sLogger.debug("=========== Applying sql updates from %s ===========", sqlUpdateDir.c_str());

        for (const auto execute : applyNewUpdateFilesStore)
        {
            const fs::path sqlFile = fs::current_path() /= execute.second.fullName;

            if (fs::exists(sqlFile))
            {
                sLogger.debug("%s", execute.second.fullName.c_str());
                std::string loadedFile = Util::readFileIntoString(sqlFile);

                // split into seperated string
                std::vector<std::string> seglist;
                std::string delimiter = ";\n";

                size_t pos = 0;
                std::string token;
                while ((pos = loadedFile.find(delimiter)) != std::string::npos)
                {
                    token = loadedFile.substr(0, pos);
                    seglist.emplace_back(token + ";");
                    loadedFile.erase(0, pos + delimiter.length());
                }

                for (const auto& statements : seglist)
                {
                    if (dbPointer.WaitExecuteNA(statements.c_str()))
                        continue;
                }
            }
        }
    }
}
