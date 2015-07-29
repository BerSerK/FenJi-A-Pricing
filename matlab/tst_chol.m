function tst_chol(matrix_file, rst_file)
    %
    A = load( matrix_file );
    chol_A = chol( A );
    chol_rst = load( rst_file );
    chol_A - chol_rst
end