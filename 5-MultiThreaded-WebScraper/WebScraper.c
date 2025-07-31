#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <curl/curl.h>

#define MAX_URLS 100

typedef struct {
    char url[2048];
    int index;
} ThreadData;

size_t to_string(void *ptr, size_t size, size_t nmemb, void *data) {
    FILE *fp = (FILE *)data;
    return fwrite(ptr, size, nmemb, fp);
}

void *fetch_url(void *arg) {
    ThreadData *tdata = (ThreadData *)arg;
    CURL *curl;
    CURLcode res;

    char filename[256];
    snprintf(filename, sizeof(filename), "output_%d.txt", tdata->index);

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        pthread_exit(NULL);
    }

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, tdata->url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "CANT FETCH URL %s: %s\n", tdata->url, curl_easy_strerror(res));
        } else {
            printf("Successful fetch %s -> %s\n", tdata->url, filename);
        }

        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failing to init curl for URL %s\n", tdata->url);
    }
    fclose(fp);
    pthread_exit(NULL);
}

int main() {
    char *urls[MAX_URLS];
    int url_count = 0;
    //My URLs are in this file
    FILE *fp = fopen("URLS.txt", "r");

    if (!fp) {
        fprintf(stderr, "cant open URL file\n");
        return 1;
    }

    // Read URLs from file
    char line[1024];
    while (fgets(line, sizeof(line), fp) && url_count < MAX_URLS) {
        line[strcspn(line, "\n")] = 0; // Remove newline
        urls[url_count] = strdup(line);
        url_count++;
    }
    fclose(fp);

    pthread_t threads[MAX_URLS];
    ThreadData thread_data[MAX_URLS];

    curl_global_init(CURL_GLOBAL_ALL);

    for (int i = 0; i < url_count; ++i) {
        strcpy(thread_data[i].url, urls[i]);
        thread_data[i].index = i;

        if (pthread_create(&threads[i], NULL, fetch_url, &thread_data[i]) != 0) {
            fprintf(stderr, "Thread creation stopped %s\n", urls[i]);
        }
    }

    for (int i = 0; i < url_count; ++i) {
        pthread_join(threads[i], NULL);
        free(urls[i]);
    }

    curl_global_cleanup();

    return 0;
}