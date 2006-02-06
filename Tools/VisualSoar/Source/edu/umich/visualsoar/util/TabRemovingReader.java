package edu.umich.visualsoar.util;
import java.io.*;


public class TabRemovingReader extends Reader 
{
    private boolean inTab = false;
    private int spaceCount = 0;
    private Reader reader;
    
    private TabRemovingReader() {}

    public TabRemovingReader(Reader inReader) 
    {
        reader = inReader;
    }
    
    public void close() throws IOException { 
        reader.close(); 
    }
 
    public void mark(int readAheadLimit) throws IOException 
    {
        throw new IOException("Mark Not Supported!");
    }
    
    public boolean markSupported() 
    {
        return false;
    }
 
    public int read() throws IOException 
    {
        if(inTab) 
        {
            spaceCount++;
            if(spaceCount < 3)
            inTab = false;
            return ' ';
        }
        int retChar = reader.read();
        if(retChar == '\t') 
        {
            inTab = true;
            spaceCount = 0;
            return ' ';
        }
        return retChar;
    }
    
    public int read(char[] cbuf) throws IOException 
    {
        for(int i = 0; i < cbuf.length; ++i) 
        {
            int retChar = read();
            if(retChar == -1) 
            {
                if(i == 0)
                return -1;
                else 
                return i;
            }
            else 
            {
                cbuf[i] = (char)retChar;
            }
        }
        return cbuf.length;
    }

    public int read(char[] cbuf, int off, int len) throws IOException 
    {
        for(int i = off; i < off+len; ++i) 
        {
            int retChar = read();
            if(retChar == -1) 
            {
                if(i == off)
                return -1;
                else 
                return i - off;
            }
            else 
            {
                cbuf[i] = (char)retChar;
            }
        }
        return len;
    }
    
    public boolean ready() throws IOException 
    {
        return reader.ready();
    }

    public void reset() throws IOException { 
        inTab = false;
        reader.reset();
    }
    
    public long skip(long n) throws IOException 
    {
        throw new IOException("Skipping Not Supported");
    }

}//class TabRemovingReader
