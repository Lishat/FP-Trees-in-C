#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define transactions 1000
#define items 27
#define max_string_size 37
#define support 0.02

//defining FP node
typedef struct FP{
    int frequency, id;
    struct FP *next, *top, *bottom[transactions];
}FP;

//defining header table node
typedef struct header_node{
    int frequency, id;
    char word[max_string_size];
    FP *next, *rear;
}header_node;

//defining frequent items
typedef struct frequent_items{
    int count;
    int frequent[transactions];
}frequent_items;

//final itemlist
char FINAL_ITEMLIST[transactions*items][items*items][max_string_size];
int frequency[transactions*items], word_count[transactions*items];
int def_count = 0;

//comparator function for quicksort
int compfunc(const void * a, const void * b){
    return strcmp(a, b);
}

//copying function for header nodes
void copy(header_node *a, header_node *b)
{
	strcpy(a->word, b->word);
    a->id = b->id;
	a->frequency = b->frequency;
	a->next = b->next;
    a->rear = b->rear;	
}
//swaping function for header nodes
void swap(header_node *distinct_words[transactions*items], int a, int b)
{
	header_node *t = (header_node *)(malloc(sizeof(header_node)));
	copy(t, distinct_words[a]);
	copy(distinct_words[a], distinct_words[b]);
    copy(distinct_words[b], t);
}
//partition function for quicksort
int Partition(header_node *distinct_words[transactions*items],int left, int right)
{
	int leftpointer = left - 1;
	int i;
    for(i = left; i <= right - 1; i++)
	{
		if(distinct_words[i]->frequency >= distinct_words[right]->frequency)	//comparing with pivot
		{
			swap(distinct_words, ++leftpointer, i);
		}
	}
	swap(distinct_words, leftpointer+1, right);		//sets pivot to correct point 
	return (leftpointer+1); 
}

//quickSort for header_nodes
void quickSort(header_node *distinct_words[transactions*items], int left, int right)
{
	if(left < right)
	{
		int part = Partition(distinct_words, left, right);
		quickSort(distinct_words, left, part-1);
		quickSort(distinct_words, part+1, right);
	}
}
//sending to quickSort recursive function
void qSort(header_node *distinct_words[transactions*items], int m)
{
    quickSort(distinct_words, 0, m-1);
}
//binarySearch in transaction table by word
int binarySearch(header_node *distinct_words[transactions*items], char find_word[max_string_size], int left, int right)
{
    int mid = left + (right - left)/2;
    if (strcmp(distinct_words[mid]->word, find_word) == 0)  
        return mid;
    if (strcmp(distinct_words[mid]->word, find_word) > 0) 
        return binarySearch(distinct_words, find_word, left, mid-1);
    if (strcmp(distinct_words[mid]->word, find_word) < 0) 
        return binarySearch(distinct_words, find_word, mid+1, right);
    return -1;
}
//binarySearch in header table by frequency
int binarySearchByFrequency(header_node *header_words[transactions*items], int frequency, int left, int right)
{
    int mid = left + (right - left)/2;
    if (header_words[mid]->frequency == frequency)
        return mid;
    if (header_words[mid]->frequency < frequency) 
        return binarySearchByFrequency(header_words, frequency, left, mid-1);
    if (header_words[mid]->frequency > frequency) 
        return binarySearchByFrequency(header_words, frequency, mid+1, right);
    return -1;
}
//binarySearch by frequency and id
int binarySearchinHeader(header_node *header_words[transactions*items], int frequency, int left, int right, int id)
{
    int approx_index  = binarySearchByFrequency(header_words, frequency, left, right);
    int i = approx_index;
    while(i <= right && header_words[i]->frequency == frequency)
    {
        if(header_words[i]->id == id)
        {
            return i;
        }
        i++;
    }
    i = approx_index;
    while(i >= left && header_words[i]->frequency == frequency)
    {
        if(header_words[i]->id == id)
        {
            return i;
        }
        i--;
    }
    return -1;
}
//recursive FP tree construction function
void constructFPTree(header_node *distinct_words[transactions*items], header_node *header_words[transactions*items], frequent_items *freq_patterns[transactions], int distinct_elements, int elements_in_header, int no_of_transactions, int total_transactions, int level, char pattern[items*items][max_string_size])
{
    FP *root = (FP *)(malloc(sizeof(FP)));
    root->id = -1;
    root->frequency = 0;
    root->next = NULL;
    root->top = NULL;
    FP *trav = root;
    int i, j, k, l, m, n;
    for(i = 0; i < elements_in_header; i++)
    {
        root->bottom[i] = NULL;
    }
    //inserting nodes in FP tree
    for(i = 0; i < no_of_transactions; i++)
    {
        trav = root;
        if(freq_patterns[i]->count > 0)
        {
            trav->frequency = trav->frequency + 1;
        }
        for(j = 0; j < freq_patterns[i]->count; j++)
        {
            int index = binarySearchinHeader(header_words, distinct_words[freq_patterns[i]->frequent[j]]->frequency, 0, elements_in_header-1, freq_patterns[i]->frequent[j]);
            if(trav->bottom[index] == NULL)
            {
                trav->bottom[index] = (FP *)(malloc(sizeof(FP)));
                trav->bottom[index]->top = trav;
                trav = trav->bottom[index];
                trav->id = freq_patterns[i]->frequent[j];
                trav->frequency = 1;
                trav->next = NULL;
                for(k = 0; k < elements_in_header; k++)
                {
                    trav->bottom[k] = NULL;
                }
                if(header_words[index]->next == NULL)
                {
                    header_words[index]->next = trav;
                    header_words[index]->rear = trav;
                    header_words[index]->rear->next = NULL;
                }
                else
                {
                    header_words[index]->rear->next = trav;
                    header_words[index]->rear = trav;
                    header_words[index]->rear->next = NULL;
                }
            }
            else
            {
                trav->bottom[index]->frequency = trav->bottom[index]->frequency + 1;
                trav = trav->bottom[index];
            }
        }
    }
    //getting all the common prefixes and sending to next recursive step
    for(i = elements_in_header-1; i >= 0; i--)
    {
        header_node *new_distinct_words[items*transactions];
        for(j = 0; j < distinct_elements; j++)
        {
            new_distinct_words[j] = (header_node *)(malloc(sizeof(header_node)));
            strcpy(new_distinct_words[j]->word, distinct_words[j]->word);
            new_distinct_words[j]->id = j;
            new_distinct_words[j]->frequency = 0;
            new_distinct_words[j]->next = NULL;
            new_distinct_words[j]->rear = NULL;
        }
        trav = header_words[i]->next;
        //printf("%s:\n", header_words[i]->word);
        frequent_items *temp_freq_pattern[transactions];
        int temp_freq_size = 0;
        while(trav != NULL)
        {
            FP *temp = trav;
            //printf("PREFIXES ");
            int common_freq = temp->frequency;
            int reversed_array[items];
            int length_reversed = 0;
            //printf("%d:%d, ", temp->id, temp->frequency);
            temp = temp->top;
            while(temp->id != -1)
            {
                //printf("%d:%d, ", temp->id, temp->frequency);
                reversed_array[length_reversed] = temp->id;
                length_reversed++;
                new_distinct_words[temp->id]->frequency = new_distinct_words[temp->id]->frequency + common_freq; 
                temp = temp->top;
            }
            //printf("COMMON:%d SIZE:%d ITERATION:%d\n", common_freq, temp_freq_size, i);
            int size_increased = 0;
            for(j = temp_freq_size; j < temp_freq_size + common_freq;j++)
            {
                if(length_reversed != 0)
                {
                    temp_freq_pattern[j] = (frequent_items *)(malloc(sizeof(frequent_items)));
                    temp_freq_pattern[j]->count = length_reversed;
                    for(k = length_reversed-1, l = 0; k >= 0; k--, l++)
                    {
                        temp_freq_pattern[j]->frequent[l] = reversed_array[k];
                    }
                    size_increased++;
                }                
            }
            temp_freq_size = temp_freq_size + size_increased;
            //printf("\n");
            trav = trav->next;
        }
        /*printf("DISTINCT TABLE:\n");
        for(j = 0; j < distinct_elements; j++)
        {
            printf("%d: %s\t-> %d\n", new_distinct_words[j]->id, new_distinct_words[j]->word, new_distinct_words[j]->frequency);
        }*/
        //constructing new header table
        header_node *new_header_words[transactions*items];
        k = 0;
        for(l = 0; l < distinct_elements; l++)
        {
            if(support*total_transactions <= new_distinct_words[l]->frequency)
            {
                new_header_words[k] = (header_node *)(malloc(sizeof(header_node)));
                copy(new_header_words[k], new_distinct_words[l]);
                k++;
            }
        }
        int new_elements_in_header = k;
        qSort(new_header_words, new_elements_in_header);
        /*printf("HEADER[%d]:\n", i);
        for(j = 0; j < new_elements_in_header; j++)
        {
            printf("%d: %s\t-> %d\n", new_header_words[j]->id, new_header_words[j]->word, new_header_words[j]->frequency);
        }*/
        //getting new frequency items
        frequent_items *new_freq_patterns[transactions];
        for(l = 0; l < temp_freq_size; l++)
        {
            new_freq_patterns[l] = (frequent_items *)malloc(sizeof(frequent_items));
            new_freq_patterns[l]->count = 0;
            for(m = 0; m < new_elements_in_header; m++)
            {
                for(n = 0; n < temp_freq_pattern[l]->count; n++)
                {
                    if(new_header_words[m]->id == temp_freq_pattern[l]->frequent[n])
                    {
                        new_freq_patterns[l]->frequent[new_freq_patterns[l]->count] = temp_freq_pattern[l]->frequent[n];
                        new_freq_patterns[l]->count = new_freq_patterns[l]->count + 1;
                    }
                }
            }
        }
        /*printf("ITERATION FOR ROUND %d & level %d:\n", i, level);
        int new_freq_size = temp_freq_size;
        for(k = 0; k < new_freq_size; k++)
        {
            printf("%d: %d||", k, new_freq_patterns[k]->count);
            for(j = 0; j < new_freq_patterns[k]->count; j++)
            {
                printf("%d ", new_freq_patterns[k]->frequent[j]);
            }
            printf("\n");
        }*/
        //getting frequent itemsets and calling next recursive itemsets
        if(header_words[i]->frequency >= support*total_transactions)
        {
            strcpy(pattern[level], header_words[i]->word);
            frequency[def_count] = header_words[i]->frequency;
            for(j = 0; j <= level; j++)
            {
                strcpy(FINAL_ITEMLIST[def_count][level-j], pattern[j]);
            }
            word_count[def_count] = level+1;
            def_count++;
            if(new_elements_in_header >= 1)
            {
                constructFPTree(new_distinct_words, new_header_words, new_freq_patterns, distinct_elements, new_elements_in_header, temp_freq_size, total_transactions, level+1, pattern);
            }
        }
    }    
}

void FPtree()
{
    char data[transactions][items][max_string_size];
    char words[transactions*items][max_string_size];
    FILE *fp = fopen("groceries_subset.csv", "r");
    char transaction[1000000];
    int i = 0,j = 0, k = 0, q = 0, r = 0, max_j = 0;
    while(fgets(transaction, 1000000, fp))
	{
		int p = 0;
        j = 0;
        while(transaction[p] != '\0' && transaction[p] != 10 && transaction[p] != 13)
        {
            if(transaction[p] != ',')
            {
                if(transaction[p] > 0)
                {
                    data[i][j][k] = transaction[p];
                    k++;
                    if(j > 0)
                    {
                        words[q][r] = transaction[p];
                        r++;
                    }
                }
            }
            else
            {
                data[i][j][k] = '\0';
                k = 0;
                j++;
                max_j = ((j > max_j)?(j):(max_j));
                if(r != 0)
                {
                    r = 0;
                    q++;
                }
            }
            p++;
        }
        data[i][j][k] = '\0';
        i++;
	}
	fclose(fp);
    header_node *distinct_words[transactions*items];
    qsort (words, q, max_string_size, compfunc);
    int l = 0, m = 0;
    do
    {
        distinct_words[m] = (header_node *)(malloc(sizeof(header_node)));
        strcpy(distinct_words[m]->word, words[l]);
        distinct_words[m]->id = m;
        distinct_words[m]->frequency = 1;
	    distinct_words[m]->next = NULL;
        distinct_words[m]->rear = NULL;
        while(l < q - 1 && strcmp(words[l], words[l+1]) == 0)
        {
            distinct_words[m]->frequency = distinct_words[m]->frequency + 1;
            l++;
        }
        l++;
        //printf("%d:%s %s %d\n", m, distinct_words[m]->word, words[l-1], distinct_words[m]->frequency);
        m++;
    }
    while(l < q);
    header_node *header_words[transactions*items];
    k = 0;
    for(l = 0; l < m; l++)
    {
        if(support*i <= distinct_words[l]->frequency)
        {
            header_words[k] = (header_node *)(malloc(sizeof(header_node)));
            copy(header_words[k], distinct_words[l]);
            k++;
        }
    }
    qSort(header_words, k);
    /*printf("%d\n", m);
    for(l = 0; l < k; l++)
    {
        printf("%d %s:\t%d\n", header_words[l]->id, header_words[l]->word, header_words[l]->frequency);
    }*/
    int elements_in_header = k, distinct_elements = m, no_of_transactions = i, no_of_columns = max_j, n;
    frequent_items *freq_patterns[i];
    for(l = 0; l < no_of_transactions; l++)
    {
        freq_patterns[l] = (frequent_items *)malloc(sizeof(frequent_items));
        freq_patterns[l]->count = 0;
        for(m = 0; m < elements_in_header; m++)
        {
            for(n = 0; n < no_of_columns; n++)
            {
                if(strcmp(header_words[m]->word, data[l][n]) == 0)
                {
                    //printf("YES:%s %s \n", header_words[m]->word, data[l][n]);
                    freq_patterns[l]->frequent[freq_patterns[l]->count] = binarySearch(distinct_words, header_words[m]->word, 0, distinct_elements - 1);
                    freq_patterns[l]->count = freq_patterns[l]->count + 1;
                }
                else
                {
                    //printf("NO:%s %s \n", header_words[m]->word, data[l][n]);
                }
            }
        }
    }
    /*for(i = 0; i < no_of_transactions; i++)
    {
        printf("%d: %d||", i, freq_patterns[i]->count);
        for(j = 0; j < freq_patterns[i]->count; j++)
        {
            printf("%s, ",distinct_words[freq_patterns[i]->frequent[j]]->word);
        }
        printf("\n");
    }*/
    char pattern[items*items][max_string_size];
    constructFPTree(distinct_words, header_words, freq_patterns, distinct_elements, elements_in_header, no_of_transactions, no_of_transactions, 0, pattern);
    printf("MAXIMUM ITEMSETS: FREQUENCY\n");
    for(i = 0; i < def_count; i++)
    {
        for(j = 0; j < word_count[i]; j++)
        {
            if(j == 0)
            {
                printf("%d. {%s",  i+1, FINAL_ITEMLIST[i][j]);
            }
            else
            {
                printf(", %s", FINAL_ITEMLIST[i][j]);
            }
            
        }
        printf("} :%d\n", frequency[i]);
    }
    printf("Total no. of frequent itemsets: %d\n", i);
}

int main()
{
    FPtree();
    return 0;
}
