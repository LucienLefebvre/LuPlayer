/*
  ==============================================================================

    FeedThread.h
    Created: 11 Feb 2022 8:05:45pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "sqlite/sqlite3.h"
#include <string>
#include "Settings.h"
//==============================================================================
/*
*/
class FeedThread  : public juce::ThreadWithProgressWindow
{
public:
    enum SoundType
    {
        None = 1,
        Speech = 2,
        Music = 3,
        Jingle = 4,
        FX = 5,
        Other = 6
    };
    struct FileToImport
    {
        juce::String path;
        juce::String name;
        juce::String name2;
        float duration;
        int type;
    };
    FeedThread() : juce::ThreadWithProgressWindow("Importing files", true, true)
    {
        importFinishedBroadcaster.reset(new juce::ChangeBroadcaster());

    }

    ~FeedThread() override
    {
    }

    void run()
    {
        setProgress(-1.0);

        rc = sqlite3_open("dataBase.db", &db);

        if (rc) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        }
        else {
            fprintf(stderr, "Opened database successfully\n");
            DBG("Database opened !");
        }

        sql = "CREATE TABLE SOUNDS("  \
            "NAME           TEXT    NOT NULL," \
            "NAME2           TEXT    NOT NULL," \
            "DURATION            REAL     NOT NULL," \
            "FILEPATH        TEXT," \
            "DATE_ADDED        CHAR(50)," \
            "TYPE         INT );";

        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        int filesNumber = filesToImport.size();

        int i = 0;
        for (auto file : filesToImport)
        {
            i++;
            setProgress(i / filesNumber);

            //COPY FILE
            juce::File currentFile(file.path);
            juce::String newFilePath = juce::File(Settings::convertedSoundsPath).getChildFile(currentFile.getFileName()).getFullPathName();
            currentFile.copyFileTo(newFilePath);

            juce::String string = "INSERT INTO SOUNDS (NAME,NAME2,DURATION,FILEPATH,DATE_ADDED,TYPE) "  
                 "VALUES ('"
                 + file.name + "', '"
                 + file.name2 + "', '"
                 + juce::String(file.duration) + "', '"
                 + newFilePath + "', DATETIME(), "
                 + juce::String(file.type) + ");";
            DBG(string);
            rc = sqlite3_exec(db, string.toRawUTF8(), callback, (void*)data, &zErrMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            else {
                fprintf(stdout, "Records created successfully\n");
            }
        }

        importFinishedBroadcaster->sendChangeMessage();
    }

    static int callback(void* data, int argc, char** argv, char** azColName)
    {
        int i;
        for (i = 0; i < argc; i++) 
        {
            if (argv[i] != nullptr)
                DBG(azColName[i] << " : " << argv[i]);
        }
        return 0;
    }

    void setFiles(juce::Array<FileToImport>& files)
    {
        filesToImport = files;
    }
    std::unique_ptr<juce::ChangeBroadcaster> importFinishedBroadcaster;
private:
    juce::Array<FileToImport> filesToImport;

    sqlite3* db;
    char* zErrMsg = 0;
    int rc;
    char* sql;
    const char* data = "Callback function called";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeedThread)
};
