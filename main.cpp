#include <sys/stat.h>
#include <fcntl.h>      /* Needed only for _O_RDWR definition */
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <share.h>
#include <Windows.h>
#include <locale>

int compare (const void* left, const void* right)
    {
    return strcmp (*(const char**) left, *(const char**) right);
    }

class Text
    {
private:
    char* buffer;
    unsigned long int nBytes = 0; // File Size
    struct stat fileStat; // Stat class to find file's size

    char** lines;
    unsigned long int nLines = 0;

public:
    Text (char filename [])
        {
        stat(filename, &fileStat);
        printf("%d bytes found\n", fileStat.st_size);

        unsigned long int bytesToLoad = fileStat.st_size;
        buffer = (char*) calloc (bytesToLoad, sizeof(char));

        int begin = GetTickCount ();

        // Opening file
        int fh = _sopen (filename, _O_RDONLY, _SH_DENYNO, 0);
        if (fh == -1)
            {
            printf ("Failed to open %s", filename);
            perror ("");
            exit(1);
            }

        // Reading lines from file
        if((nBytes = _read (fh, buffer, bytesToLoad)) == -1)
            {
            switch(errno)
                {
                case EBADF:
                    perror("Bad file descriptor!");
                    break;
                case ENOSPC:
                    perror("No space left on device!");
                    break;
                case EINVAL:
                    perror("Invalid parameter: buffer was NULL!");
                    break;
                default:
                    perror("Unexpected error!");
                }
            }
        else
            printf ("Read %u bytes from file\n", nBytes);

        // Closing file
        _close (fh);

        // Clearing buffer's end if necessary
        for (unsigned int i = fileStat.st_size-1; nBytes <= i; i--)
            buffer [i] = '\0';

        // Counting lines & replacing \n with \0
        // Checking for
        nLines = 1;
        for (int i = 0; i < fileStat.st_size; i++)
            {
            if (buffer [i] == '\n')
                {
                buffer [i] = '\0';
                nLines++;
                }
            }

        // Filling pointer-to-char array &
        // Adding first char to array &
        // Setting iterator to '1' (looking for second element)
        lines  = (char**) calloc (nLines, sizeof(char*));
        lines [0] = &buffer [0];
        int line = 1;
        for (int i = 0; i < fileStat.st_size && line < nLines; i++)
            {
            if (buffer [i] == '\0')
                {
                lines [line] = &buffer [i+1];
                line++;
                }
            }

        printf ("File load took %d ms\nLoaded %d lines\n-----------------------\n", (int) GetTickCount() - begin, nLines);
        }

    ~Text ()
        {
        }

    void sort ()
        {
        int begin = GetTickCount ();
        qsort (lines, nLines, sizeof (lines [0]), compare);

        printf ("File sort took %d ms\n-----------------------\n", (int) GetTickCount() - begin);
        }

    void print ()
        {
        printf ("File print:\n\n");
        for (int i = 0; i < nLines; i++)
            printf ("%s\n", lines [i]);
        printf ("-----------------------\n");
        }

    void save (char filename [])
        {
        int begin = GetTickCount ();

        char* buf = new char [nBytes];
        unsigned long int coursorPos = 0;

        // Filling buffer in sort consequence
        for (int i = 0; i < nLines; i++)
            {
            for (int j = 0;; j++)
                {
                char* c = lines[i]+j;
                buf [coursorPos + j] = *c;
                if (!*c)
                    {
                    buf [coursorPos + j] = '\n';
                    coursorPos += j+1;
                    break;
                    }
                }
            }

        // Opening file
        int fh =  _sopen (filename, _O_RDWR | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);
        if (!(fh))
            exit (-1);

        // Friting buffer's data to file
        unsigned int writtenToFile = _write(fh, buf, nBytes);
        if (writtenToFile == -1)
            {
            switch(errno)
                {
                 case EBADF:
                    perror("Bad file descriptor");
                    break;
                 case ENOSPC:
                    perror("No space left");
                    break;
                 case EINVAL:
                    perror("Invalid parameter");
                    break;
                 default:
                    perror("Unexpected error");
                }
            }
        else
            printf ("Wrote %u bytes to file.\n", writtenToFile);

        // Closing File
        _close(fh);

        printf ("File save took %d ms\n-----------------------\n", (int) GetTickCount() - begin);
        }

    void freeMemory ()
        {
        free (buffer);
        free (lines);
        }


    };

// Note: "Валентин" = "William S."
//       "Женек" = "Evgeniy Onegin"

int main()
    {
    setlocale (LC_ALL, "Russian");

    char filename []       = "TestFiles\\Валентин.txt";
    char filenameToSave [] = "TestFiles\\save.txt";

    Text tx (filename);
    tx.sort ();
    tx.save (filenameToSave);
    tx.freeMemory();
    tx.~Text();

    return 0;
    }
