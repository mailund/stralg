
./match_test naive test-data/modest-proposal.txt > test-data/naive-match.txt
./match_test kmp test-data/modest-proposal.txt > test-data/kmp-match.txt
./match_test bmh test-data/modest-proposal.txt > test-data/bmh-match.txt

if [ cmp test-data/naive-match.txt test-data/kmp-match.txt ];
then
    echo "naive and KMP disagree"
    exit 1
fi

if [ cmp test-data/naive-match.txt test-data/bmh-match.txt ];
then
    echo "naive and BMH disagree"
    exit 1
fi
