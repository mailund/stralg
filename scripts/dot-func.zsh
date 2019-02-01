
func showdot() {
    dotfile=$1
    if (( $# == 1 )); then
        pdffile=`basename $dotfile .dot`.pdf
    else
        pdffile=$2
    fi
    dot -Tpdf < $dotfile > $pdffile && open $pdffile
}

for i in {0..11}; do
    showdot v_$i.dot v.pdf
    showdot parent_$i.dot parent.pdf
    read -k1 -s
done

