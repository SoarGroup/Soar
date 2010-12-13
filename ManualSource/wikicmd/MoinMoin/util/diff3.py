# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - diff3 algorithm

    @copyright: 2002 Florian Festi
    @license: GNU GPL, see COPYING for details.
"""

default_markers = ('<<<<<<<<<<<<<<<<<<<<<<<<<\n',
                   '=========================\n',
                   '>>>>>>>>>>>>>>>>>>>>>>>>>\n')

def text_merge(old, other, new, allow_conflicts=1, *markers):
    """ do line by line diff3 merge with three strings """
    result = merge(old.splitlines(1), other.splitlines(1), new.splitlines(1),
                   allow_conflicts, *markers)
    return ''.join(result)

def merge(old, other, new, allow_conflicts=1, *markers):
    """ do line by line diff3 merge
        input must be lists containing single lines
    """
    if not markers:
        markers = default_markers
    marker1, marker2, marker3 = markers

    old_nr, other_nr, new_nr = 0, 0, 0
    old_len, other_len, new_len = len(old), len(other), len(new)
    result = []

    while old_nr < old_len and other_nr < other_len and new_nr < new_len:
        # unchanged
        if old[old_nr] == other[other_nr] == new[new_nr]:
            result.append(old[old_nr])
            old_nr += 1
            other_nr += 1
            new_nr += 1
        else:
            if allow_conflicts == 2: # experimental addition to the algorithm
                if other[other_nr] == new[new_nr]:
                    result.append(new[new_nr])
                    other_nr += 1
                    new_nr += 1
                    continue
            new_match = find_match(old, new, old_nr, new_nr)
            other_match = find_match(old, other, old_nr, other_nr)
            # new is changed
            if new_match != (old_nr, new_nr):
                new_changed_lines = new_match[0] - old_nr
                # other is unchanged
                if match(old, other, old_nr, other_nr,
                         new_changed_lines) == new_changed_lines:
                    result.extend(new[new_nr:new_match[1]])
                    old_nr = new_match[0]
                    new_nr = new_match[1]
                    other_nr += new_changed_lines
                # both changed, conflict!
                else:
                    if not allow_conflicts:
                        return None
                    old_m, other_m, new_m = tripple_match(
                        old, other, new, other_match, new_match)
                    result.append(marker1)
                    result.extend(other[other_nr:other_m])
                    result.append(marker2)
                    result.extend(new[new_nr:new_m])
                    result.append(marker3)
                    old_nr, other_nr, new_nr = old_m, other_m, new_m
            # other is changed
            else:
                other_changed_lines = other_match[0] - other_nr
                # new is unchanged
                if match(old, new, old_nr, new_nr,
                         other_changed_lines) == other_changed_lines:
                    result.extend(other[other_nr:other_match[1]])
                    old_nr = other_match[0]
                    other_nr = other_match[1]
                    new_nr += other_changed_lines
                # both changed, conflict!
                else:
                    if not allow_conflicts:
                        return None
                    old_m, other_m, new_m = tripple_match(
                        old, other, new, other_match, new_match)
                    result.append(marker1)
                    result.extend(other[other_nr:other_m])
                    result.append(marker2)
                    result.extend(new[new_nr:new_m])
                    result.append(marker3)
                    old_nr, other_nr, new_nr = old_m, other_m, new_m

    # process tail
    # all finished
    if old_nr == old_len and other_nr == other_len and new_nr == new_len:
        pass
    # new added lines
    elif old_nr == old_len and other_nr == other_len:
        result.extend(new[new_nr:])
    # other added lines
    elif old_nr == old_len and new_nr == new_len:
        result.extend(other[other_nr:])
    # new deleted lines
    elif (new_nr == new_len and (old_len - old_nr == other_len - other_nr) and
          match(old, other, old_nr, other_nr, old_len-old_nr) == old_len - old_nr):
        pass
    # other deleted lines
    elif (other_nr == other_len and (old_len - old_nr == new_len-new_nr) and
          match(old, new, old_nr, new_nr, old_len-old_nr) == old_len - old_nr):
        pass
    # conflict
    else:
        if new == other:
            result.extend(new[new_nr:])
        else:
            if not allow_conflicts:
                return None
            result.append(marker1)
            result.extend(other[other_nr:])
            result.append(marker2)
            result.extend(new[new_nr:])
            result.append(marker3)
    return result

def tripple_match(old, other, new, other_match, new_match):
    """find next matching pattern unchanged in both other and new
       return the position in all three lists
    """
    while 1:
        difference = new_match[0] - other_match[0]
        # new changed more lines
        if difference > 0:
            match_len = match(old, other, other_match[0], other_match[1],
                              difference)
            if match_len == difference:
                return (new_match[0], other_match[1]+difference, new_match[1])
            else:
                other_match = find_match(old, other,
                                         other_match[0] + match_len,
                                         other_match[1] + match_len)
        # other changed more lines
        elif difference < 0:
            difference = -difference
            match_len = match(old, new, new_match[0], new_match[1],
                              difference)
            if match_len == difference:
                return (other_match[0], other_match[1],
                        new_match[0] + difference)
            else:
                new_match = find_match(old, new,
                                       new_match[0] + match_len,
                                       new_match[1] + match_len)
        # both conflicts change same number of lines
        # or no match till the end
        else:
            return (new_match[0], other_match[1], new_match[1])

def match(list1, list2, nr1, nr2, maxcount=3):
    """ return the number matching items after the given positions
        maximum maxcount lines are are processed
    """
    i = 0
    len1 = len(list1)
    len2 = len(list2)
    while nr1 < len1 and nr2 < len2 and list1[nr1] == list2[nr2]:
        nr1 += 1
        nr2 += 1
        i += 1
        if i >= maxcount and maxcount > 0:
            break
    return i

def find_match(list1, list2, nr1, nr2, mincount=3):
    """searches next matching pattern with lenght mincount
       if no pattern is found len of the both lists is returned
    """
    len1 = len(list1)
    len2 = len(list2)
    hit1 = None
    hit2 = None
    idx1 = nr1
    idx2 = nr2

    while (idx1 < len1) or (idx2 < len2):
        i = nr1
        while i <= idx1:
            hit_count = match(list1, list2, i, idx2, mincount)
            if hit_count >= mincount:
                hit1 = (i, idx2)
                break
            i += 1

        i = nr2
        while i < idx2:
            hit_count = match(list1, list2, idx1, i, mincount)
            if hit_count >= mincount:
                hit2 = (idx1, i)
                break
            i += 1

        if hit1 or hit2:
            break
        if idx1 < len1:
            idx1 += 1
        if idx2 < len2:
            idx2 += 1

    if hit1 and hit2:
        #XXX which one?
        return hit1
    elif hit1:
        return hit1
    elif hit2:
        return hit2
    else:
        return (len1, len2)

def main():

    text0 = """AAA 001
AAA 002
AAA 003
AAA 004
AAA 005
AAA 006
AAA 007
AAA 008
AAA 009
AAA 010
AAA 011
AAA 012
AAA 013
AAA 014
"""

    text1 = """AAA 001
AAA 002
AAA 005
AAA 006
AAA 007
AAA 008
BBB 001
BBB 002
AAA 009
AAA 010
BBB 003
"""

    text2 = """AAA 001
AAA 002
AAA 003
AAA 004
AAA 005
AAA 006
AAA 007
AAA 008
CCC 001
CCC 002
CCC 003
AAA 012
AAA 013
AAA 014
"""


    text = text_merge(text0, text1, text2)
    print(text)

if __name__ == '__main__':
    main()
