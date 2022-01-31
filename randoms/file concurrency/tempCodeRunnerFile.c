int main()
{
    FILE *fin = fopen(FILENAME, "w");

    for(int i = 0; i < strlen(SENTENCE); i++)
    {
        if (fork() == 0)
        {
            sleep(1); // to make sure child acts first
            printf("child  => %c : %ld\n", fprintf(fin,"%c", SENTENCE[i]), ftell(fin));
            exit(0);
        }
        else
        {
            sleep(1); // to make sure child acts first
            printf("parent => %c : %ld\n", fprintf(fin,"%c", SENTENCE[i]), ftell(fin));
        }
    }

    return 0;
}