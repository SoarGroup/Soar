# towers-of-hanoi/README
# John Laird
# March 8, 1999
#
# ABSTRACT. This file provides a Soar system to solve the Tower 
# of Hanoi problems. This puzzle "involves three vertical pegs or 
# posts and a number of doughnut-like disks of graduated sizes that
# fit  on the pegs. At the outset, all the disks are arranged pyrami-
# dally on one of the pegs, say A, with the largest disk on the bottom. 
# The task is to move all of the disks to another peg, C, say, under 
# the constraints that (1) only one disk may be moved at a time, and 
# (2) a disk may never be placed on top of another smaller than itself. 
# Any number of disks may be used; the minimum number of moves for
# a solution is (2**n - 1), where n is the number of disks" (Simon, 
# 1975/1979, pp. 230-231).
#
# 
# Towers of Hanoi puzzle
#    Two strategies implemented - no search or learning.
#    Multiple implementations of one of the strategies using 
#    different representations with different generality and different
#    execution costs
#
# towers-of-hanoi-recursive.soar
#    Uses the disk recursive strategy to solve the towers-of-hanoi.
#    Tries to always moves the biggest out of place disk into
#     its correct position
#
# monitor.soar: Monitoring rules for tracing movement of disks
#    for towers-of-hanoi.soar
