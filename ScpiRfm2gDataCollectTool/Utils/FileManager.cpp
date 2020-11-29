#include <Windows.h>
#include "FileManager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

string FileManager::getAppDir()
{
    //»ñÈ¡AppÂ·¾¶
	char modulePath[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);

    string appDir = modulePath;
    int pos = appDir.rfind('\\');
    appDir = appDir.substr(0, pos + 1);

    return appDir;
}


void FileManager::removeFile( QString filePath )
{
    QFile file;
    file.remove(filePath);
}

void FileManager::removeDir( QString dirPath )
{
    QString path = dirPath;
    if (path.contains("\\")) {
        path.replace("\\", "/");
    } 

    QDir dir(path);  
    if(!dir.exists()){  
        return;  
    }  
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);  
    QFileInfoList fileList = dir.entryInfoList();  
    foreach (QFileInfo file, fileList) {  
        if (file.isFile()){
            file.dir().remove(file.fileName());  
        } else{ // µÝ¹éÉ¾³ý  
            removeDir(file.absoluteFilePath());  
        }  
    }   
    dir.rmdir(path); // É¾³ýÎÄ¼þ¼Ð  
}

void FileManager::removeDirContent( QString dirPath )
{
    QString path = dirPath;
    if (path.contains("\\")) {
        path.replace("\\", "/");
    }

    QDir dir(path);  
    if(!dir.exists()){  
        return;  
    }  
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);  
    QFileInfoList fileList = dir.entryInfoList();  
    foreach (QFileInfo file, fileList) {  
        if (file.isFile()){
            file.dir().remove(file.fileName());  
        } else{ // µÝ¹éÉ¾³ý  
            removeDir(file.absoluteFilePath());  
        }  
    }   
}

QString FileManager::createDir(QString dirPath)
{
    QString path = dirPath;
    if (path.contains("\\")) {
        path.replace("\\", "/");
    }
    
    QDir dir(path);
    if (dir.exists(path)){
        return path;
    }
    QString parentDir = createDir(path.mid(0, path.lastIndexOf('/')));
    QString dirname = path.mid(path.lastIndexOf('/') + 1);
    QDir parentPath(parentDir);
    if (!dirname.isEmpty()) {
        parentPath.mkpath(dirname);
    }

    return parentDir + "/" + dirname;
}

bool FileManager::renameDir( QString oldDirPath, QString newDirName )
{
    QDir dir;
    if (!dir.exists(oldDirPath)){
        return false;
    }

    QString path = oldDirPath;
    if (path.contains("\\")) {
        path.replace("\\", "/");
    }

    QString parentPath = path.left(path.lastIndexOf('/') + 1);
    if (parentPath.isEmpty()) {
        return false;
    }

    QString newDirPath = parentPath + newDirName;
    return dir.rename(oldDirPath, newDirPath);    
}

QString FileManager::removeSuffix( QString path )
{
    //ÒÆ³ýºó×º
	return path.left(path.lastIndexOf('.'));
}

QStringList FileManager::getFilePathsOfDir( QString dirPath, QString suffix, bool needSort /*= true*/ )
{
    QStringList filePaths;
    QDir dir(dirPath);
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        QString path = fileInfo.absoluteFilePath();
        if (path.endsWith(suffix)) {
            filePaths << path;
        }
    }

    if (needSort) {
        filePaths.sort();
    }

    return filePaths;
}