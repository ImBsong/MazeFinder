#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <fstream>

namespace MazeLog {

    class FileLogger {

    public:


        // If you can´t/dont-want-to use C++11, remove the "class" word after enum
        enum class e_logType { LOG_ERROR, LOG_WARNING, LOG_INFO };


        // ctor 
        FileLogger() {}

        // Initializier 
        void initFile()
        {

            numWarnings = 0U;
            numErrors = 0U;

            const char * engine_version = "1.1";
            const char * fname = "maze_log.txt";

            myFile.open(fname);
            if (myFile.is_open()) {
                myFile << "MazeFinder, version " << engine_version << std::endl;
                myFile << "Log file created" << std::endl << std::endl;
            } // if
        }

        // dtor
        ~FileLogger() {

            if (myFile.is_open()) {
                myFile << std::endl << std::endl;

                // Report number of errors and warnings
                myFile << numWarnings << " warnings" << std::endl;
                myFile << numErrors << " errors" << std::endl;

                myFile.close();
            } // if

        }


        // Overload << operator using log type
        friend FileLogger &operator << (FileLogger &logger, const e_logType l_type) {

            switch (l_type) {
            case MazeLog::FileLogger::e_logType::LOG_ERROR:
                logger.myFile << "[ERROR]: ";
                ++logger.numErrors;
                break;

            case MazeLog::FileLogger::e_logType::LOG_WARNING:
                logger.myFile << "[WARNING]: ";
                ++logger.numWarnings;
                break;
            case MazeLog::FileLogger::e_logType::LOG_INFO:
                logger.myFile << "[INFO]: ";
                break;
            default:
                logger.myFile << "[INFO]: ";
                break;
            } // sw


            return logger;

        }


        // Overload << operator using C style strings
        friend FileLogger &operator << (FileLogger &logger, const char *text) {

            logger.myFile << text << std::endl;
            return logger;

        }

        friend FileLogger &operator << (FileLogger &logger, std::string text)
        {
            logger.myFile << text << std::endl;
            return logger;
        }


        // Make it Non Copyable (or you can inherit from sf::NonCopyable if you want)
        FileLogger(const FileLogger &) = delete;
        FileLogger &operator= (const FileLogger &) = delete;



    private:

        std::ofstream           myFile;

        unsigned int            numWarnings;
        unsigned int            numErrors;

    }; // class end

}  // namespace

#endif // !FILELOGGER_H





