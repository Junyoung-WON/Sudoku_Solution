#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define SIZE 9

/*
���� (������ ������� ������ ��)
	1. ����� ���ڰ� ����϶�. - ��
	2. ���������� ���� �� �� �ִ� cell�� ���ڸ� ä�� 1�� ����� ���
	3. ��� cell�� ���� �Ҵ��� ���� ����� ���
	4. �ٸ� ��� ������ �ش䵵 ���
*/

//���� �̸��� �Է¹޾�, �ش� �̸��� ������ �о� ������ ��� ������ �迭�� �����Ѵ�.
//fopen, fscanf�Լ� �̿�

//0�� �� ������ ��ġ�� �����ϴ� ������ ����� -> ����ü�� ������. 0�� �ִ� �ڸ��� ��,���� �����ϴ� ����ü �迭.
//���� �ϳ��ۿ� �� �� ���� cell�� ��� ä���
//�� ���Ŀ� backtracking�� Ȱ���Ͽ� ������ ��� ĭ���� ä���.

typedef struct { //������ �迭���� cell�� ��, �� ��ġ�� �����ϴ� ����ü Cell. 
	int row;
	int col;
}Cell;

int sudoku[SIZE][SIZE] = { 0 };	//������ ����� �����ϴ� 2���� �迭. 
int data_length = 0;			//������ ����(������ ����)�� ����.
Cell* zero_cells;				//0�� �� ������ ��ġ����(row, col)�� �����ϴ� ����ü�迭 zero_cells ����.
int max_z;						//zero_cells�� ���̸� ����.

//Cell* Stack[81];
//int top = -1;
int count_ans = 0;

void Print();
void Read_File(FILE*);
void find_zero();
void print_zero_index();
int count_val();
void del_candidates();
void del_rowcol();
void check_answer(int);
//void push(int row, int col);
//void pop();

int main() {


	char fname[20];
	printf("���� �̸��� �Է��ϼ���(.txt�� �Բ� �Է�) >>> ");
	gets_s(fname, 20);
	FILE* fp;
	fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("%s ���� �б� ����! ���� �̸��� �ǰ� �Է��ߴ��� Ȯ���غ�����.\n", fname);
		return 0;
	}
	
	Read_File(fp);
	printf("\n������ ���� ��� ���\n");
	Print();

	max_z = 81 - data_length;
	zero_cells = (Cell*)malloc(max_z * sizeof(Cell));
	find_zero();
	//print_zero_index();


	printf("\n�ĺ� �Ұ� �Լ� ����\n");
	del_candidates();
	printf("\n�ĺ� �Ұ� ���� ���\n");
	Print();


	data_length = count_val();
	max_z = 81 - data_length;
	zero_cells = (Cell*)realloc(zero_cells, max_z * sizeof(Cell));
	find_zero();


	printf("\n�ش� ���!\n");
	check_answer(0);

	printf("\n�� %d���� �ش��� ��µǾ����ϴ�.\n", count_ans);

	free(zero_cells);
	fclose(fp);
}

void Print() {
	printf("�������������������������� SUDOKU!��������������������������\n");
	for (int row = 0; row < SIZE; row++) {
		if (row == 3 || row == 6) printf("��������������������������������������������������������������������\n");
		for (int col = 0; col < SIZE; col++) {
			if (col % 3 == 0) printf("�� ");
			if (sudoku[row][col] == 0)
				printf(" - ");
			else
				printf(" %d ", sudoku[row][col]);
		}
		printf("��\n");
	}
	printf("��������������������������������������������������������������������\n");
}

void Read_File(FILE* fp) {
	fscanf(fp, "%d", &data_length);
	printf("data_length : %d\n\n", data_length);

	int r = 0;
	int c = 0;
	int val = 0;
	for (int i = 0; i < data_length; i++) {
		fscanf(fp, "%d %d %d", &r, &c, &val);
		sudoku[r][c] = val;
	}
}

//�� ��(0�� �ִ� �ڸ�)�� ã�� ����ü �迭 zero_cell�� ��ġ ����(��, ��)�� �����ϴ� �Լ�
//�� �켱 ������ �����
void find_zero() {
	int index = 0;
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (sudoku[i][j] == 0) {
				(zero_cells + index)->row = i;
				(zero_cells + index)->col = j;
				index++;
			}
		}
	}
}
void print_zero_index() {
	printf("zero index\nrow\tcol\n");
	for (int i = 0; i < 81 - data_length; i++) {
		printf(" %d\t %d\n", (zero_cells + i)->row, (zero_cells + i)->col);
	}
}
int count_val() {
	int count = 0;
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (sudoku[i][j] != 0) {
				count++;
			}
		}
	}
	return count;
}

/* �� �Լ� ���� �����ϰ�
* �ڽ� ������ �� ������ Ȯ��, �� ������ 1 ~ 9���� ���� �ĺ����� ������, üũ�� �ϸ鼭 �� �ĺ����� �Ұ��ϴ� ���.
* ������ ���� 1���� ������ �� ���ڸ� �� ���� ����.
* �ĺ��� �� ������ ���� �� ���� ���� �迭�� ���� {1, 2, ... , 9}
	�ĺ� �迭�� ������ �ʱ�ȭ�� �ϴ� ���� �޸� ������ ���ؼ��� �� ������?

* �켱 �ڽ��ȿ� �ִ� ���ڵ��� ��� �� ������ �ĺ����� ����(0���� ����).
* �ش� ���� ��, ���� �˻��ؼ� ���ڰ� �ִٸ� �� ���ڸ� �ĺ����� ����.
* �ش� ���� ���� �ĺ��� �ϳ��ۿ� �������� �ʴٸ� �� ���ڸ� �� �� ��ġ�� �ִ´�.
*/
void del_candidates() {	//�ĺ� �Ұ� �Լ�
	int count_can = 0;	//�ĺ��� ������ �����ϴ� ����
	int final_c;		//���� �ĺ��� ���� �����ϴ� ����
	int change = 0;		//�ĺ� ���� �� ���� �ѹ��̶� �־����� ǥ���ϴ� ���� 
	int candidate[9];	//�� ���� �ĺ����

	for (int index = 0; index < max_z; index++) {

		count_can = 0;	//�ĺ��� ������ 0���� �ʱ�ȭ.
		int r = (zero_cells + index)->row / 3 * 3;
		int c = (zero_cells + index)->col / 3 * 3;
		for (int i = 0; i < 9; i++)	//�ĺ� �迭�� { 1,2,3, ... ,9 }�� �ʱ�ȭ
			candidate[i] = i + 1;

		//�ش� �ڽ� �ȿ� ���ڵ��� �ĺ���ϰ� ���Ͽ� �ĺ����� ����
		for (int a = r; a < r + 3; a++) {
			for (int b = c; b < c + 3; b++) {
				for (int k = 0; k < 9; k++) {
					if (sudoku[a][b] != 0 && candidate[k] == sudoku[a][b])
						candidate[k] = 0;
				}
			}
		}
		// �� ���� ��� ���� �ĺ��� ���� ���ڰ� �ִٸ� �ĺ����� ����
		for (int i = 0; i < SIZE; i++) {
			for (int k = 0; k < 9; k++) {
				if (sudoku[(zero_cells + index)->row][i] != 0 && candidate[k] == sudoku[(zero_cells + index)->row][i])
					candidate[k] = 0;
			}
			for (int k = 0; k < 9; k++) {
				if (sudoku[i][(zero_cells + index)->col] != 0 && candidate[k] == sudoku[i][(zero_cells + index)->col])
					candidate[k] = 0;
			}
		}
		// 0�� �ƴ� �ĺ��� ������ ī��Ʈ, 0�� �ƴ� �ĺ��� �켱 ���� �ĺ� final_c�� ����
		for (int i = 0; i < 9; i++)
			if (candidate[i] != 0) {
				count_can++;
				final_c = candidate[i];
			}
		// �ĺ��� ������ 1�����̶��, ���� �ĺ� ���� �ش� �� ���� ����.
		if (count_can == 1) {
			sudoku[(zero_cells + index)->row][(zero_cells + index)->col] = final_c;
			change++;
		}
		else
			continue;
	}
	//printf("�󼿿� ���� Ƚ��: %d\n", change);
	if (change == 0)	//�ĺ� ���� �ѹ��� �� ���� ���� �ʾ��� ��� ����.
		return;
	del_candidates();	//�� ���� ���� �Ҵ��� ���� ���� �� ���� �ĺ� �Ұ� �Լ��� �ݺ�
}

/* ���� �� �Լ��� �����ϸ� �ɵ�
* ���������� �ڽ������� ��
* 1���� �����ؼ� 9���� ������� ���ڸ� üũ.
* �ش� ���ڰ� �����ϴ� ��, ���� �񱳿��� ����(����)
* ���� �� ���� 1����� �ش� ���ڸ� �� ���� ����

* �켱 �� ��� ���ڰ� �ڽ� ���� �ִ��� Ȯ��.
* ���ٸ� �ش� ���ڸ� �ٹ�(�ڽ� ���� ��) ��� ������ ã�� ��, ���ڰ� ���� ��� ���� �񱳿��� ����
* ���� �ڽ� ���� �� ��(�� ���� �ε����� Ư�� ���� �ȿ� ��ٸ�?) �� ���ڰ� ���� ��� ���� �����ϰ� ���� �� ���� ������ 1�� ���̶�� �ش� ���ڸ� �� ���� ����.
*/

//�� �ȵǳ�,,,,
void del_rowcol() {
	int r, c;
	int row_del[2], col_del[2];
	int cnt_r, cnt_c, cnt_zero;
	int change = 0;
	int row = -1, col = -1;

	for (int box_r = 0; box_r < 9; box_r += 3) {
		for (int box_c = 0; box_c < 9; box_c += 3) {
			r = box_r / 3 * 3;
			c = box_c / 3 * 3;

			for (int key = 1; key <= 9; key++) {	//1���� 9���� ������� üũ
				cnt_r = 0;
				cnt_c = 0;
				cnt_zero = 0;

				for (int a = r; a < r + 3; a++) {	//�ش� �ڽ� �ȿ� ���� ���ڰ� �ִ��� ��
					for (int b = c; b < c + 3; b++) {
						if (sudoku[a][b] == key) {
							goto NEXT_NUM;	//�ִٸ� NEXT_NUM ���̺�� �� �� ���� ���ڸ� ��.
						}
					}
				}

				//���� ���ڰ� ���ٸ� ��, ���� �� ����
				for (int a = r; a < r + 3; a++) {	//�� ��
					for (int i = 0; i < 9; i++)
						if (sudoku[a][i] == key) {
							row_del[cnt_r] = a;
							cnt_r++;
						}
				}
				for (int b = c; b < c + 3; b++) {	//�� ��
					for (int i = 0; i < 9; i++)
						if (sudoku[i][b] == key) {
							col_del[cnt_c] = b;
							cnt_c++;
						}
				}

				//�� ĭ���� ��
				for (int idx = 0; idx < max_z; idx++) {
					int i = 0, j = 0, next = 0;
					if ((r < (zero_cells + idx)->row < r + 3) && (c < (zero_cells + idx)->col < c + 3)) {	//�� ĭ�� �ش� �ڽ� �ȿ� ������
						for (i = 0; i < cnt_r; i++) {	//���ܵ� ��� ��ĭ�� ���� ��
							if ((zero_cells + idx)->row == row_del[i])	//���ٸ� for�� �����Ͽ� ���� ��ĭ�� ��
								break;
							cnt_zero++;		//�ٸ� ��� ��ĭ�� ���� 1 ����
							row = (zero_cells + idx)->row;	//��ĭ�� ���� row�� ����
							break;
						}
							for (j = 0; j < cnt_c; j++) {	//���ܵ� ���� ��ĭ�� ���� ��
								if ((zero_cells + idx)->col == col_del[j])	//���ٸ� for�� �����Ͽ� ���� ��ĭ�� ��
									break;
								cnt_zero++;		//�ٸ� ��� ��ĭ�� ���� 1����
								col = (zero_cells + idx)->col;	//��ĭ�� ���� col�� ����
								break;
							}
					
						//for (row = r; row < r + 3; row++) {
						//	if (row == row_del[i]) {	//���ܵ� ��� ������ ���� �� üũ
						//		i++;
						//		continue;
						//	}
						//	break;	//���ܵ� ��� �ٸ��� for�� Ż��
						//}
						//for (col = c; col < c + 3; col++) {
						//	if (col == col_del[j]) {	//���ܵ� ���� ������ ���� �� üũ
						//		j++;
						//		continue;
						//	}
						//	break;	//���ܵ� ��� �ٸ��� for�� Ż��
						//}
						//if (sudoku[row][col] == 0) {	//sudoku[row][col]�� ���� ��ĭ�̸� key ����.
						//	sudoku[row][col] = key;
						//	change++;
						//	goto EXIT;	//key�� ���� �� EXIT ���̺�� ���� ���� �ڽ��� üũ.
						//}
					}
				}
				if (cnt_zero == 1) {
					sudoku[row][col] = key;
					change++;
				}
				
				row_del[0] = -1, row_del[1] = -1;
				col_del[0] = -1, col_del[1] = -1;

			NEXT_NUM:
				continue;
			}
		}
	}
	printf("�󼿿� ���� Ƚ��: %d\n", change);
	if (change == 0)	//���� �ѹ��� �� ���� ���� �ʾ��� ��� ����.
		return;
	del_rowcol();
}

//0�� �ִ� �� ��ġ�� 1~9���� ���ڸ� �־ Ȯ���ϴ� check_answer. 
//�ش� ���� ���� ��, ���� ���� ���ڰ� �ִ��� üũ ��
//3*3 �ڽ� �ϳ� ������ �ڽ� �ȿ� ���� ���ڰ� �ִ��� üũ.
//���� ���ڰ� �ִٸ� ���� ���ڸ� üũ
//���� ���ڰ� ���ٸ� ���ڸ� �ְ� ���� ���� �Ѿ�� �� üũ.
//�ش� �� ���� Ȯ���� �� �ֵ��� index�� �Ű������� �ְ�, ���� �� Ȯ���� ���� ���� check_answer�� index + 1�� �ֱ�
//��� �ݺ��ϴٰ� ���� ������ ������ �������� ���ư��� backtracking(����Լ��̿�)
//�̰� �� �ǳ�,,,
void check_answer(int index) {
	
	if (index == max_z) {
		count_ans++;
		printf("\n\t  < %d��° �ش� >\n", count_ans);
		Print();
		return;
	}

	bool same;	//�Ȱ��� ���ڰ� �������� �˸��� bool.
	for (int val = 1; val <= SIZE; val++) {	//���ڸ� 1 ���� 9���� �־�� for �ݺ���.
		same = false;						//same�� false�� �ʱ�ȭ
		//��, ���� �˻�
		for (int k = 0; k < SIZE; k++) {
			if (sudoku[(zero_cells + index)->row][k] == val) {	//�ش� �� ĭ�� �࿡�� �������� ���ڿ� ���� ���ڰ� ������
				same = true;		//���� ������ �˸��� same�� true�� ����
				break;
			}
		}
		if (same == true)	//val�� ���� ���� �ִٸ� �н��ϰ� �Ѿ�� ���� ���ڸ� üũ
			continue;
		for (int k = 0; k < SIZE; k++) {
			if (sudoku[k][(zero_cells + index)->col] == val) {	//�ش� �� ĭ�� ������ �������� ���ڿ� ���� ���ڰ� ������
				same = true;			//���� ������ �˸��� same�� true�� ����
				break;
			}
		}
		if (same == true)	//val�� ���� ���ڰ� �ִٸ� �Ѿ�� ���� ���ڸ� üũ
			continue;
		//�ڽ� ���� �˻�
		int r = (zero_cells + index)->row / 3 * 3;
		int c = (zero_cells + index)->col / 3 * 3;
		for (int a = r; a < r + 3; a++) {
			for (int b = c; b < c + 3; b++) {
				if (sudoku[a][b] == val)	//���� ���ڰ� �ش� �ڽ����� ������
					same = true;			//���� ������ �˸��� same�� true�� ����.
			}
		}
		if (same == true)	// val�� ���� ���ڰ� �ִٸ� ���� ���ڸ� üũ
			continue;
		sudoku[(zero_cells + index)->row][(zero_cells + index)->col] = val;
		check_answer(index + 1);
		sudoku[(zero_cells + index)->row][(zero_cells + index)->col] = 0;
			//��Ʈ��ŷ
			//���� �־����?

			/*sudoku[row][col] = val;
			push(row, col);

			check_answer();

			sudoku[row][col] = 0;
			pop();*/

	}

}

//void push(int row, int col) {
//	if (top >= 80) {
//		printf("Stack is Full!\n");
//		return;
//	}
//	Stack[++top]->row = row;
//	Stack[++top]->col = col;
//}
//
//void pop() {
//	if (top == -1) {
//		printf("Stack is Empty!\n");
//		return;
//	}
//	Stack[top--];
//}