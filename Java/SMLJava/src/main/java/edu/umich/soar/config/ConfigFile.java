package edu.umich.soar.config;

import java.io.*;
import java.util.*;

/** Concrete implementation of ConfigSource using a file. **/
public class ConfigFile extends ConfigSource
{
    String path;

    String prefixPath;

    HashMap<String, String[]> keys = new HashMap<String, String[]>();

    public ConfigFile(String path) throws IOException
    {
        this.path = path;
        File f = new File(path);
        this.prefixPath = f.getParent() + File.separator;

        Tokenizer t = new Tokenizer(path);
        parse(t, "");
    }

    public ConfigFile()
    {
        this.path = "(in memory)";
        this.prefixPath = "";
    }

    public Config getConfig()
    {
        return new Config(this);
    }

    public String getPrefixPath()
    {
        return prefixPath;
    }

    public String[] getKeys(String root)
    {
        if (!root.equals(""))
            root += ".";

        ArrayList<String> subkeys = new ArrayList<String>();

        for (String key : keys.keySet())
        {
            if (key.length() <= root.length())
                continue;
            if (key.startsWith(root))
            {
                subkeys.add(key.substring(root.length()));
            }
        }

        return subkeys.toArray(new String[subkeys.size()]);
    }

    public boolean hasKey(String key)
    {
        return keys.get(key) != null;
    }

    public int[] getInts(String key)
    {
        String vs[] = keys.get(key);
        if (vs == null)
            return null;

        int v[] = new int[vs.length];
        for (int i = 0; i < vs.length; i++)
        {
            v[i] = Integer.parseInt(vs[i]);
        }

        return v;
    }

    public void setInts(String key, int v[])
    {
        String s[] = new String[v.length];
        for (int i = 0; i < v.length; i++)
            s[i] = "" + v[i];
        keys.put(key, s);
    }

    public boolean[] getBooleans(String key)
    {
        String vs[] = keys.get(key);
        if (vs == null)
            return null;

        boolean v[] = new boolean[vs.length];
        for (int i = 0; i < vs.length; i++)
        {
            v[i] = Boolean.parseBoolean(vs[i]);
        }

        return v;
    }

    public void setBooleans(String key, boolean v[])
    {
        String s[] = new String[v.length];
        for (int i = 0; i < v.length; i++)
            s[i] = "" + v[i];
        keys.put(key, s);
    }

    public String[] getStrings(String key)
    {
        String vs[] = keys.get(key);
        if (vs == null)
            return null;

        return vs;
    }

    public void setStrings(String key, String v[])
    {
        keys.put(key, v);
    }

    public double[] getDoubles(String key)
    {
        String vs[] = keys.get(key);
        if (vs == null)
            return null;

        double v[] = new double[vs.length];
        for (int i = 0; i < vs.length; i++)
        {
            v[i] = Double.parseDouble(vs[i]);
        }

        return v;
    }

    public void setDoubles(String key, double v[])
    {
        String s[] = new String[v.length];
        for (int i = 0; i < v.length; i++)
            s[i] = "" + v[i];
        keys.put(key, s);
    }

    public byte[] getBytes(String key)
    {
        String lines[] = getStrings(key);
        if (lines == null)
            return null;

        return Base64.decode(lines);
    }

    public void setBytes(String key, byte v[])
    {
        keys.put(key, Base64.encode(v));
    }

    // ///////////////////////////////////////////////////////
    // File parsing below

    void parseError(Tokenizer t, String msg)
    {
        System.out.println("Parse error: " + msg);
        System.out.println("Near line " + t.lineNumber + ": " + t.line);
    }

    void parse(Tokenizer t, String keyroot) throws IOException
    {
        while (true)
        {

            if (!t.hasNext())
                return;

            if (t.consume("{"))
            {
                // an anonymous enclosure? Anonymous enclosures are
                // used for scoping inheritance without introducing
                // extra namespace segments into the path.
                parse(t, keyroot);
            }

            // end of block?
            if (t.consume("}"))
            {
                if (keyroot.equals(""))
                    parseError(t, "Unmatched } in input");

                return;
            }

            if (!t.hasNext())
                return;

            // parse a key block.
            String keypart = t.next();

            if (!t.hasNext())
            {
                parseError(t, "Premature EOF");
                return;
            }

            // we have an enclosure block?
            if (t.consume("{"))
            {
                parse(t, keyroot + keypart + ".");
                continue;
            }

            // This is a key/value declaration.
            String tok = t.next();
            if (!tok.equals("="))
            {
                parseError(t, "Expected = got " + tok);
                return;
            }

            ArrayList<String> values = new ArrayList<String>();

            if (t.consume("["))
            {
                // read a list of values
                while (true)
                {
                    tok = t.next();
                    if (tok.equals("]"))
                        break;
                    values.add(tok);
                    tok = t.peek();
                    if (tok.equals(","))
                        t.next();
                }
            }
            else
            {
                // read a single value
                values.add(t.next());
            }

            if (!t.consume(";"))
                parseError(t, "Expected ; got " + tok);

            String key = keyroot + keypart;

            if (keys.get(key) != null)
            {
                parseError(t, "Duplicate key definition for: " + key);
            }

            keys.put(key, values.toArray(new String[0]));

            /*
             * System.out.println(keyroot+keypart+" = "); for (String s :
             * values) System.out.println("  "+s);
             */
        }
    }

    class Tokenizer
    {
        BufferedReader ins;

        // tokens belonging to the current line
        String line;

        int lineNumber = 0;

        LinkedList<String> tokens = new LinkedList<String>();

        public Tokenizer(String path) throws IOException
        {
            ins = new BufferedReader(new FileReader(path));
        }

        // doesn't support string literals spread across multiple lines.
        void tokenizeLine(String line)
        {
            String TOKSTOP = "[];{},=#";

            String tok = "";
            boolean in_string = false;

            for (int pos = 0; pos < line.length(); pos++)
            {
                char c = line.charAt(pos);

                if (in_string)
                {
                    // in a string literal

                    if (c == '\\' && pos + 1 < line.length())
                    {
                        // escape sequence.
                        tok += line.charAt(pos + 1);
                        pos++;
                        continue;
                    }

                    if (c == '\"')
                    {
                        // end of string.
                        tokens.addLast(tok);
                        in_string = false;
                        tok = "";
                        continue;
                    }

                    tok += c;

                }
                else
                {
                    // NOT in a string literal

                    // strip spaces when not in a string literal
                    if (Character.isWhitespace(c))
                        continue;

                    // starting a string literal
                    if (c == '\"' && tok.length() == 0)
                    {
                        in_string = true;
                        continue;
                    }

                    // does this character end a token?
                    if (TOKSTOP.indexOf(c) < 0)
                    {
                        // nope, add it to our token so far
                        tok += c;
                        continue;
                    }

                    // produce (up to) two new tokens: the accumulated token
                    // which has just ended, and a token corresponding to the
                    // new character.
                    tok = tok.trim();
                    if (tok.length() > 0)
                    {
                        tokens.addLast(tok);
                        tok = "";
                    }

                    if (c == '#')
                        return;

                    // add this terminator character
                    tok = "" + c;
                    tok = tok.trim();
                    if (tok.length() > 0)
                    {
                        tokens.addLast(tok);
                        tok = "";
                    }
                }
            }

            tok = tok.trim();
            if (tok.length() > 0)
                tokens.addLast(tok);

        }

        public boolean hasNext() throws IOException
        {
            while (true)
            {
                if (tokens.size() > 0)
                    return true;

                line = ins.readLine();
                lineNumber++;
                if (line == null)
                    return false;

                tokenizeLine(line);
            }
        }

        // If the next token is s, consume it.
        public boolean consume(String s) throws IOException
        {
            if (peek().equals(s))
            {
                next();
                return true;
            }
            return false;
        }

        public String peek() throws IOException
        {
            if (!hasNext())
                return null;

            return tokens.peek();
        }

        public String next() throws IOException
        {
            if (!hasNext())
                return null;

            String tok = tokens.poll();
            return tok;
        }
    }

    // ///////////////////////////////////////////////////////
    // Simple testing harness

    public static void main(String args[])
    {
        try
        {
            ConfigFile cf = new ConfigFile(args[0]);

            String path = cf.getConfig().getPath(
                    "simulator.obstacle_image.path");
            System.out.println(path);

            path = cf.getConfig().getPath("viewer.avatar.path");
            System.out.println(path);

            path = cf.getConfig().getPath("viewer.avatar_lr3.path");
            System.out.println(path);

        }
        catch (IOException ex)
        {
            System.out.println("ex: " + ex);
        }
    }
}
