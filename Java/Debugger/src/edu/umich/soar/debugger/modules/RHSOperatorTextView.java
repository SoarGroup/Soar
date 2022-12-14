package edu.umich.soar.debugger.modules;

import java.util.HashSet;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import sml.Agent;

public class RHSOperatorTextView extends RHSObjectTextView
{
    public String getModuleBaseName()
    {
        return "rhs_operator_text";
    }

    enum ParseState
    {
        START, ACCEPTABLES, BEFORE_NUM, NUM, DONE
    }

    @Override
    protected void updateNow()
    {
        if (clear)
        {
            this.clearDisplay();
            clear = false;
        }

        // String test1 = "  O154 (verify) =0 :I";
        // String test2 = "  O154 (verify) =0.244948 :I";
        // String test3 = "  O153 (clear-attended-flags) =-0.0745079 :I";
        // String test4 = "  O1 (two) + :I ";
        // Matcher m;
        // m = Pattern.compile("^\\s+(O\\d+).+=(.+)\\s.*$").matcher(test1);
        // if (m.matches()) {
        // System.out.println(m.group(2));
        // }
        // m = Pattern.compile("^\\s+(O\\d+).+=(.+)\\s.*$").matcher(test2);
        // if (m.matches()) {
        // System.out.println(m.group(2));
        // }
        // m = Pattern.compile("^\\s+(O\\d+).+=(.+)\\s.*$").matcher(test3);
        // if (m.matches()) {
        // System.out.println(m.group(2));
        // }
        // m = Pattern.compile("^\\s+(O\\d+)\\s+.*").matcher(test4);
        // if (m.matches()) {
        // System.out.println(m.group(1));
        // }
        // System.out.println("end");
        //
        // if (true) return;

        if (this.idToOrdered.size() < 1)
        {
            return;
        }

        Agent agent = m_Frame.getAgentFocus();
        if (agent == null)
        {
            return;
        }

        // clear out old values
        {
            for (OrderedIdentifier oid : idToOrdered.values()) {
                oid.resetValues();
            }
        }

        HashSet<String> reported = new HashSet<>();

        // get preferences result, split on newlines
        String[] prefOutput = agent.ExecuteCommandLine("pref s1").split("\n");

        // get preference value for each operator
        ParseState state = ParseState.START;
        Matcher matcher;
        String match1;
        String match2;
        for (String s : prefOutput) {
            switch (state) {
                case START:
                    matcher = Pattern.compile("acceptables:$").matcher(
                        s);
                    if (matcher.matches()) {
                        state = ParseState.ACCEPTABLES;
                    }
                    break;

                case ACCEPTABLES:
                    matcher = Pattern.compile("^\\s+(O\\d+)\\s+.*").matcher(
                        s);
                    if (matcher.matches()) {
                        match1 = matcher.group(1);

                        OrderedIdentifier oid = this.idToOrdered.get(match1);
                        if (oid != null) {
                            reported.add(match1);
                        }
                    } else {
                        state = ParseState.BEFORE_NUM;
                    }
                    break;

                case BEFORE_NUM:
                    matcher = Pattern.compile(".*indifferents:$").matcher(
                        s);
                    if (matcher.matches()) {
                        state = ParseState.NUM;
                    }
                    break;

                case NUM:
                    matcher = Pattern.compile(
                        "^\\s+(O\\d+).+=(-?\\d+\\.?\\d*)\\s.*$").matcher(
                        s);
                    if (matcher.matches()) {
                        match1 = matcher.group(1);
                        match2 = matcher.group(2);

                        OrderedIdentifier oid = this.idToOrdered.get(match1);
                        if (oid != null) {
                            double value = oid.getDoubleOrder();
                            value += Double.parseDouble(match2);
                            // System.out.println(oid + " --> " + value);

                            oid.setDoubleOrder(value);
                        }
                    } else {
                        state = ParseState.DONE;
                    }
                    break;

                default:
                    assert false;
            }

            if (state == ParseState.DONE) {
                break;
            }
        }

        this.idToOrdered.keySet().retainAll(reported);
        this.sortedIdentifiers.clear();
        {
            for (Map.Entry<String, OrderedIdentifier> stringOrderedIdentifierEntry : this.idToOrdered
                .entrySet()) {
                this.sortedIdentifiers.add(stringOrderedIdentifierEntry.getValue());
            }
        }

        // iterate through sortedIdentifiers and print values
        StringBuilder output = new StringBuilder();
        {
            for (OrderedIdentifier oid : this.sortedIdentifiers) {
                output.append(oid.getIdentifier());
                output.append(" (");
                output.append(oid.getDoubleOrder());
                output.append(")\n");
                output.append(oid.getAttributes());

            }
        }

        setTextSafely(output.toString());
    }
}
