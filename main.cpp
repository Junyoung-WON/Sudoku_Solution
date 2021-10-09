#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define SIZE 9

/*
문제 (무조건 순서대로 이행할 것)
	1. 행렬을 예쁘게 출력하라. - 완
	2. 직관적으로 값을 알 수 있는 cell에 숫자를 채운 1차 결과를 출력
	3. 모든 cell에 값을 할당한 최종 행렬을 출력
	4. 다른 모든 가능한 해답도 출력
*/

//파일 이름을 입력받아, 해당 이름의 파일을 읽어 파일의 행렬 정보를 배열에 저장한다.
//fopen, fscanf함수 이용

//0이 들어간 셀들의 위치를 저장하는 변수를 만든다 -> 구조체로 만들자. 0이 있는 자리의 행,열을 저장하는 구조체 배열.
//값이 하나밖에 들어갈 수 없는 cell을 모두 채운다
//그 이후에 backtracking을 활용하여 나머지 모든 칸들을 채운다.

typedef struct { //스도쿠 배열에서 cell의 행, 열 위치를 저장하는 구조체 Cell. 
	int row;
	int col;
}Cell;

int sudoku[SIZE][SIZE] = { 0 };	//스도쿠 행렬을 저장하는 2차원 배열. 
int data_length = 0;			//데이터 길이(숫자의 개수)를 저장.
Cell* zero_cells;				//0이 들어간 셀들의 위치정보(row, col)을 저장하는 구조체배열 zero_cells 생성.
int max_z;						//zero_cells의 길이를 저장.

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
	printf("파일 이름을 입력하세요(.txt도 함께 입력) >>> ");
	gets_s(fname, 20);
	FILE* fp;
	fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("%s 파일 읽기 실패! 파일 이름을 옳게 입력했는지 확인해보세요.\n", fname);
		return 0;
	}
	
	Read_File(fp);
	printf("\n스도쿠 문제 행렬 출력\n");
	Print();

	max_z = 81 - data_length;
	zero_cells = (Cell*)malloc(max_z * sizeof(Cell));
	find_zero();
	//print_zero_index();


	printf("\n후보 소거 함수 실행\n");
	del_candidates();
	printf("\n후보 소거 이후 결과\n");
	Print();


	data_length = count_val();
	max_z = 81 - data_length;
	zero_cells = (Cell*)realloc(zero_cells, max_z * sizeof(Cell));
	find_zero();


	printf("\n해답 출력!\n");
	check_answer(0);

	printf("\n총 %d개의 해답이 출력되었습니다.\n", count_ans);

	free(zero_cells);
	fclose(fp);
}

void Print() {
	printf("┌──────────┬─ SUDOKU!─┬──────────┐\n");
	for (int row = 0; row < SIZE; row++) {
		if (row == 3 || row == 6) printf("├──────────┼──────────┼──────────┤\n");
		for (int col = 0; col < SIZE; col++) {
			if (col % 3 == 0) printf("│ ");
			if (sudoku[row][col] == 0)
				printf(" - ");
			else
				printf(" %d ", sudoku[row][col]);
		}
		printf("│\n");
	}
	printf("└──────────┴──────────┴──────────┘\n");
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

//빈 셀(0이 있는 자리)을 찾아 구조체 배열 zero_cell에 위치 정보(행, 열)를 저장하는 함수
//행 우선 순서로 저장됨
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

/* 이 함수 먼저 실행하고
* 박스 단위로 빈 셀들을 확인, 빈 셀마다 1 ~ 9까지 숫자 후보들이 있으며, 체크를 하면서 그 후보들을 소거하는 방식.
* 최후의 숫자 1개가 남으면 그 숫자를 빈 셀에 지정.
* 후보는 매 셀들을 비교할 때 마다 새로 배열을 지정 {1, 2, ... , 9}
	후보 배열의 값들을 초기화를 하는 것이 메모리 공간에 대해서는 더 좋을듯?

* 우선 박스안에 있는 숫자들은 모든 빈 셀들의 후보에서 제외(0으로 지정).
* 해당 셀의 행, 열을 검사해서 숫자가 있다면 그 숫자를 후보에서 제외.
* 해당 셀이 가진 후보가 하나밖에 존재하지 않다면 그 숫자를 빈 셀 위치에 넣는다.
*/
void del_candidates() {	//후보 소거 함수
	int count_can = 0;	//후보의 개수를 저장하는 변수
	int final_c;		//최종 후보의 값을 저장하는 변수
	int change = 0;		//후보 값을 빈 셀에 한번이라도 넣었음을 표시하는 변수 
	int candidate[9];	//빈 셀의 후보목록

	for (int index = 0; index < max_z; index++) {

		count_can = 0;	//후보의 개수를 0으로 초기화.
		int r = (zero_cells + index)->row / 3 * 3;
		int c = (zero_cells + index)->col / 3 * 3;
		for (int i = 0; i < 9; i++)	//후보 배열을 { 1,2,3, ... ,9 }로 초기화
			candidate[i] = i + 1;

		//해당 박스 안에 숫자들을 후보목록과 비교하여 후보에서 제외
		for (int a = r; a < r + 3; a++) {
			for (int b = c; b < c + 3; b++) {
				for (int k = 0; k < 9; k++) {
					if (sudoku[a][b] != 0 && candidate[k] == sudoku[a][b])
						candidate[k] = 0;
				}
			}
		}
		// 빈 셀의 행과 열에 후보와 같은 숫자가 있다면 후보에서 제외
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
		// 0이 아닌 후보의 개수를 카운트, 0이 아닌 후보를 우선 최종 후보 final_c에 저장
		for (int i = 0; i < 9; i++)
			if (candidate[i] != 0) {
				count_can++;
				final_c = candidate[i];
			}
		// 후보의 개수가 1개뿐이라면, 최종 후보 값을 해당 빈 셀에 삽입.
		if (count_can == 1) {
			sudoku[(zero_cells + index)->row][(zero_cells + index)->col] = final_c;
			change++;
		}
		else
			continue;
	}
	//printf("빈셀에 넣은 횟수: %d\n", change);
	if (change == 0)	//후보 값을 한번도 빈 셀에 넣지 않았을 경우 종료.
		return;
	del_candidates();	//빈 셀에 값이 할당이 되지 않을 때 까지 후보 소거 함수를 반복
}

/* 다음 이 함수를 실행하면 될듯
* 마찬가지로 박스단위로 비교
* 1부터 시작해서 9까지 순서대로 숫자를 체크.
* 해당 숫자가 존재하는 행, 열은 비교에서 제외(삭제)
* 남은 빈 셀이 1개라면 해당 숫자를 빈 셀에 지정

* 우선 비교 대상 숫자가 박스 내에 있는지 확인.
* 없다면 해당 숫자를 근방(박스 범위 내) 행과 열에서 찾은 뒤, 숫자가 나온 행과 열을 비교에서 제외
* 만약 박스 내의 빈 셀(빈 셀의 인덱스가 특정 범위 안에 든다면?) 중 숫자가 나온 행과 열을 제거하고 남은 빈 셀의 개수가 1개 뿐이라면 해당 숫자를 빈 셀에 삽입.
*/

//왜 안되노,,,,
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

			for (int key = 1; key <= 9; key++) {	//1부터 9까지 순서대로 체크
				cnt_r = 0;
				cnt_c = 0;
				cnt_zero = 0;

				for (int a = r; a < r + 3; a++) {	//해당 박스 안에 같은 숫자가 있는지 비교
					for (int b = c; b < c + 3; b++) {
						if (sudoku[a][b] == key) {
							goto NEXT_NUM;	//있다면 NEXT_NUM 레이블로 간 후 다음 숫자를 비교.
						}
					}
				}

				//같은 숫자가 없다면 행, 열과 비교 시작
				for (int a = r; a < r + 3; a++) {	//행 비교
					for (int i = 0; i < 9; i++)
						if (sudoku[a][i] == key) {
							row_del[cnt_r] = a;
							cnt_r++;
						}
				}
				for (int b = c; b < c + 3; b++) {	//열 비교
					for (int i = 0; i < 9; i++)
						if (sudoku[i][b] == key) {
							col_del[cnt_c] = b;
							cnt_c++;
						}
				}

				//빈 칸들을 비교
				for (int idx = 0; idx < max_z; idx++) {
					int i = 0, j = 0, next = 0;
					if ((r < (zero_cells + idx)->row < r + 3) && (c < (zero_cells + idx)->col < c + 3)) {	//빈 칸이 해당 박스 안에 있으면
						for (i = 0; i < cnt_r; i++) {	//제외된 행과 빈칸의 행을 비교
							if ((zero_cells + idx)->row == row_del[i])	//같다면 for문 종료하여 다음 빈칸을 비교
								break;
							cnt_zero++;		//다를 경우 빈칸의 수를 1 증가
							row = (zero_cells + idx)->row;	//빈칸의 행을 row에 저장
							break;
						}
							for (j = 0; j < cnt_c; j++) {	//제외된 열과 빈칸의 열을 비교
								if ((zero_cells + idx)->col == col_del[j])	//같다면 for문 종료하여 다음 빈칸을 비교
									break;
								cnt_zero++;		//다를 경우 빈칸의 수를 1증가
								col = (zero_cells + idx)->col;	//빈칸의 열을 col에 저장
								break;
							}
					
						//for (row = r; row < r + 3; row++) {
						//	if (row == row_del[i]) {	//제외된 행과 같으면 다음 행 체크
						//		i++;
						//		continue;
						//	}
						//	break;	//제외된 행과 다르면 for문 탈출
						//}
						//for (col = c; col < c + 3; col++) {
						//	if (col == col_del[j]) {	//제외된 열과 같으면 다음 열 체크
						//		j++;
						//		continue;
						//	}
						//	break;	//제외된 행과 다르면 for문 탈출
						//}
						//if (sudoku[row][col] == 0) {	//sudoku[row][col]의 셀이 빈칸이면 key 저장.
						//	sudoku[row][col] = key;
						//	change++;
						//	goto EXIT;	//key를 저장 후 EXIT 레이블로 가서 다음 박스를 체크.
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
	printf("빈셀에 넣은 횟수: %d\n", change);
	if (change == 0)	//값을 한번도 빈 셀에 넣지 않았을 경우 종료.
		return;
	del_rowcol();
}

//0이 있는 셀 위치에 1~9까지 숫자를 넣어서 확인하는 check_answer. 
//해당 셀과 같은 행, 열에 같은 숫자가 있는지 체크 후
//3*3 박스 하나 단위로 박스 안에 같은 숫자가 있는지 체크.
//같은 숫자가 있다면 다음 숫자를 체크
//같은 숫자가 없다면 숫자를 넣고 다음 셀로 넘어가서 또 체크.
//해당 빈 셀을 확인할 수 있도록 index를 매개변수로 넣고, 다음 셀 확인을 위해 다음 check_answer에 index + 1을 넣기
//계속 반복하다가 답이 나오지 않으면 이전으로 돌아가는 backtracking(재귀함수이용)
//이게 왜 되노,,,
void check_answer(int index) {
	
	if (index == max_z) {
		count_ans++;
		printf("\n\t  < %d번째 해답 >\n", count_ans);
		Print();
		return;
	}

	bool same;	//똑같은 숫자가 나왔음을 알리는 bool.
	for (int val = 1; val <= SIZE; val++) {	//숫자를 1 부터 9까지 넣어보는 for 반복문.
		same = false;						//same을 false로 초기화
		//행, 열을 검사
		for (int k = 0; k < SIZE; k++) {
			if (sudoku[(zero_cells + index)->row][k] == val) {	//해당 빈 칸의 행에서 넣으려는 숫자와 같은 숫자가 나오면
				same = true;		//수가 같음을 알리는 same을 true로 설정
				break;
			}
		}
		if (same == true)	//val과 같은 수가 있다면 패스하고 넘어가서 다음 숫자를 체크
			continue;
		for (int k = 0; k < SIZE; k++) {
			if (sudoku[k][(zero_cells + index)->col] == val) {	//해당 빈 칸의 열에서 넣으려는 숫자와 같은 숫자가 나오면
				same = true;			//수가 같음을 알리는 same을 true로 설정
				break;
			}
		}
		if (same == true)	//val과 같은 숫자가 있다면 넘어가서 다음 숫자를 체크
			continue;
		//박스 안을 검사
		int r = (zero_cells + index)->row / 3 * 3;
		int c = (zero_cells + index)->col / 3 * 3;
		for (int a = r; a < r + 3; a++) {
			for (int b = c; b < c + 3; b++) {
				if (sudoku[a][b] == val)	//같은 숫자가 해당 박스에서 나오면
					same = true;			//수가 같음을 알리는 same을 true로 설정.
			}
		}
		if (same == true)	// val과 같은 숫자가 있다면 다음 숫자를 체크
			continue;
		sudoku[(zero_cells + index)->row][(zero_cells + index)->col] = val;
		check_answer(index + 1);
		sudoku[(zero_cells + index)->row][(zero_cells + index)->col] = 0;
			//백트래킹
			//스택 있어야함?

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