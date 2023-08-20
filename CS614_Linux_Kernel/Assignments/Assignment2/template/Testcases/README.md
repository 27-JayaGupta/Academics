## Testcase Details

Testcases are available in testcases directory, use these programs to evaluate your Assignment-2 submission. You need to run these programs and check testcase pass/fail as mentioned below. We have provided 1 line description of each testcase at the top of corresponding testcase file.

tc1 to tc10 Part 1.1, Total 1 marks per testcase
tc11 to tc20 Part 1.2, Total 1 marks per testcase
tc21 to tc30 Part 2, Total 4 marks per testcase
tc31 to tc40 Part 3, Total 4 marks per testcase


These testcases print pass or fail based on the scenario of the testcase, except for tc21 to tc30 and tc38, tc40.

=================
**For tc21 to tc30 and tc38, tc40**

Directory module\_ptw contains kernel module to print **pmd** and **pte** values for testcases tc21 to tc30, tc38 and tc40
compile and insert **ptextract.ko** while running these testcases. 

for tc21 to tc30, **pmd** and **pte** values are printed **before promotion** and **after promotion**. You need to compare the pmd values before and after to determine pass or fail of a testcase. 

We have used python scripts to parse the testcase outputs for tc21 to tc30, tc38 and tc40. We are sharing those scripts for your reference, modify the script with testcase output filename as required.

for tc38, tc40 **pmd** and **pte** values are printed **before compaction** and **after compaction** .You need to compare the pmd values before and after to determine pass or fail of a testcase.

