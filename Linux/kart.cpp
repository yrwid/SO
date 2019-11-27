
#include <stdio.h>
#include <stdlib.h>
/*
int main(void)
{
	unsigned long a[10] = { 7,5,3,1 };
	unsigned char b[10] = { 7,5,3,1 };

	printf("0x%1x\n", (unsigned long)a);
	printf("0x%1x\n", (unsigned long)sizeof(*a));
	printf("0x%1x\n", (unsigned long)b);
	printf("---\n");
	printf("00: 0x%1x\n", (unsigned long)*a);
	printf("01: 0x%1x\n", (unsigned long)*b);
	printf("02: 0x%1x\n", (unsigned long)a[1]);
	printf("03: 0x%1x\n", (unsigned long)b[1]);
	printf("04: 0x%1x\n", (unsigned long)(a + 1));
	printf("05: 0x%1x\n", (unsigned long)(b + 1));
	printf("06: 0x%1x\n", (unsigned long)(*a + 1));
	printf("07: 0x%1x\n", (unsigned long)(*b + 1));
	printf("08: 0x%1x\n", (unsigned long)&a[1]);
	printf("09: 0x%1x\n", (unsigned long)&b[1]);
	printf("10: 0x%1x\n", (unsigned long)*(a + 1));
	printf("11: 0x%1x\n", (unsigned long)*(b + 1));
	printf("12: 0x%1x\n", (unsigned long)1[a]);

	//system("dir");
	system("pause");
	return 0;
}
*/

/*int main()
{
	FILE *plik;
	char tekst[] = "siemka";
	plik = fopen("testowe.txt", "w");
	if (plik == 0)
	{
		printf("Nie da sie otworzyc pliku\n");
		exit(1);
	}
	fprintf(plik, "%s", tekst);
	fclose(plik);
	system("pause");
	return 0;
}z
/*
#include <stdio.h>
#include <stdlib.h>

struct foo{
	unsigned long v;
	unsigned char data[10];
};

int main(void)
{
	struct foo a[3] = {
		{6, {1,2}},
	{7, {3,4,5}}
	};

	printf("0x%1x\n", (unsigned long)a);
	printf("0x%1x\n", (unsigned long)sizeof(*a));
	printf("0x%1d\n", (unsigned long)sizeof(a[0].v));
	//printf("0x%1d\n", (unsigned long)sizeof(a[1].data));
	//printf("0x%1x\n", (unsigned long)sizeof(a[2]));
	//printf("0x%1x\n", (unsigned long)sizeof(a[0].v));
	//printf("0x%1x\n", (unsigned long)sizeof(a[0].data));
	//printf("0x%1x\n", (unsigned long) sizeof(a));
	//printf("0x%1x\n", (unsigned long)&a[0].v);
	printf("0x%1x\n", (unsigned long)a[0].data);
	printf("---\n");
	printf("00: 0x%1x\n", (unsigned long) (a+1));
	printf("01: 0x%1x\n", (unsigned long)&a[1]);
	//printf("00: 0x%1x\n", (unsigned long)a[11]);
	printf("02: 0x%1x\n", (unsigned long)a[1].v);
	printf("03: 0x%1x\n", (unsigned long)a[1].data);
	printf("04: 0x%1x\n", (unsigned long)&a[1].v);
	printf("05: 0x%1x\n", (unsigned long)&a[1].data);
	printf("06: 0x%1x\n", (unsigned long)a[1].data[1]);
	printf("06bis: 0x%1x\n", (unsigned long)a[1].data[1]);
	printf("07: 0x%1x\n", (unsigned long)&a[1].data[1]);
	printf("08: 0x%1x\n", (unsigned long)(a[1].data+1));
	printf("09: 0x%1x\n", (unsigned long)(*(a[1].data +1)));

	//system("dir");
	system("pause");
	return 0;
} */
/*
struct list {
	struct list *next;
	int v;
};

static void add(struct list prev, struct list next)
{
	prev.next = &next;
}

int main(void)
{
	struct list a[5] = { {a + 3,3},{a,4} };
	unsigned int i;

	printf("00:  0x%1x\n", (unsigned long)&a[0]);
	printf("10:  0x%1x\n", (unsigned long)&a[1]);
	printf("20:  0x%1x\n", (unsigned long)&a[2]);
	printf("30:  0x%1x\n", (unsigned long)&a[3]);
	printf("40:  0x%1x\n", (unsigned long)&a[4]);
	printf("---\n");
	printf("00:  0x%1x\n", (unsigned long)a[0].next);
	printf("10:  0x%1x\n", (unsigned long)a[1].next);
	printf("20:  0x%1x\n", (unsigned long)a[2].next);
	printf("30:  0x%1x\n", (unsigned long)a[3].next);
	printf("40:  0x%1x\n", (unsigned long)a[4].next);

	for (i = 0; i < 4; i++) {
		add(a[i], a[i + 1]);
	}
	printf("---\n");
	printf("---\n");
	printf("00:  0x%1x\n", (unsigned long)&a[0]);
	printf("10:  0x%1x\n", (unsigned long)&a[1]);
	printf("20:  0x%1x\n", (unsigned long)&a[2]);
	printf("30:  0x%1x\n", (unsigned long)&a[3]);
	printf("40:  0x%1x\n", (unsigned long)&a[4]);
	printf("---\n");
	printf("00:  0x%1x\n", (unsigned long)a[0].next);   
	printf("10:  0x%1x\n", (unsigned long)a[1].next);
	printf("20:  0x%1x\n", (unsigned long)a[2].next);
	printf("30:  0x%1x\n", (unsigned long)a[3].next);
	printf("40:  0x%1x\n", (unsigned long)a[4].next);



	printf("0x%1x\n", (unsigned long)a);
	printf("0x%1x\n", (unsigned long)sizeof(unsigned long));
	printf("0x%1x\n", (unsigned long)sizeof(*a));
	printf("0x%1x\n", (unsigned long)sizeof(a[0].next));
	printf("---\n");
	printf("00: 0x%1x\n", (unsigned long)(a + 4));
	printf("01: 0x%1x\n", (unsigned long)a[1].next);
	printf("02: 0x%1x\n", (unsigned long)a[1].next->v);
	printf("03: 0x%1x\n", (unsigned long)a[1].next->next->v);
	printf("04: 0x%1x\n", (unsigned long)a[3].next);
	printf("05: 0x%1x\n", (unsigned long)a[0].next->next);
	printf("06: 0x%1x\n", (unsigned long)a[1].next->next);
	printf("07: 0x%1x\n", (unsigned long)*(unsigned long *)&a[1]);
	printf("08: 0x%1x\n", (unsigned long)*(unsigned long *)&a[3]);
	//printf("09: 0x%1x\n", (unsigned long) 1[a]);
	
	system("pause");
	return 0;
}
*/
struct list {
	struct list *prev;
	struct list *nest;
	int v;
};
int main(void)
{
	unsigned int a[10] = { 3,4,5,7 };
	struct list x;
	struct list y = { &x, &x, 2 };
	x ={ &y, &y, 1 };
	printf("0x%1x\n", (unsigned long)a);
	printf("0x%1x\n", (unsigned long)sizeof(*a));
	printf("0x%1x\n", (unsigned long)&x);
	printf("0x%1x\n", (unsigned long)&x.v);
	printf("0x%1x\n", (unsigned long)&y);
	printf("0x%1x\n", (unsigned long)&y.v);
	printf("0x%1x\n", (unsigned long)&x.prev);
	printf("---\n");
	printf("00: 0x%1x\n", (unsigned long)(a + 1));
	printf("01: 0x%1x\n", (unsigned long)*(a+1));
	printf("02: 0x%1x\n", (unsigned long)a[1]);
	printf("03: 0x%1x\n", (unsigned long)&a[1]);
	printf("04: 0x%1x\n", (unsigned long)x.prev);
	printf("05: 0x%1x\n", (unsigned long)&(x.prev));
	printf("06: 0x%1x\n", (unsigned long)x.prev->v);
	printf("07: 0x%1x\n", (unsigned long)(*x.prev).v);
	printf("08: 0x%1x\n", (unsigned long)&(x.prev->v));
	//printf("09: 0x%1x\n", (unsigned long)x.next->next->v);
	//printf("09: 0x%1x\n", (unsigned long)1[a]);
	system("pause");
	return 0;
} 