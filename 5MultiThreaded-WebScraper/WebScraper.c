#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
//curl needed for webscraping and pthread for parallelism

//SSStruct
typedef struct {
    char *url;
    char *filename;
} ThreadData;

//Callback func to write recieved data into file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

//Thread func to fetch the actual URL
void *fetch_url(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    CURL *curl_handle;
    CURLcode res;
    FILE *fp;

    printf("Thread from URL: %s. Saving to: %s\n", data->url, data->filename);

    //curl session to activate to run the program for the URl collection
    curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error: Something is wrong with the URL %s\n", data->url);
        pthread_exit(NULL);
    }

    // Open the file to write the content
    fp = fopen(data->filename, "wb"); // Use "wb" for binary write, suitable for HTML
    if (!fp) {
        fprintf(stderr, "Error: Could not open file %s for writing for URL %s\n", data->filename, data->url);
        curl_easy_cleanup(curl_handle);
        pthread_exit(NULL);
    }

    //Curl options for handling
    curl_easy_setopt(curl_handle, CURLOPT_URL, data->url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);             //File pointer to callback
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);        //Follows redirections

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "Can't fetch URL %s: %s\n", data->url, curl_easy_strerror(res));
    } else {
        printf("Successfully got URL: %s and saved to %s\n", data->url, data->filename);
    }

    //Clean up
    fclose(fp);
    curl_easy_cleanup(curl_handle);

    //Free memory allocated to the file handling
    free(data->url);
    free(data->filename);
    free(data);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    //Cehecks if URL are in the command line as arguments
    if (argc < 2) {
        fprintf(stderr, "URL isnt as a command line argument\n", argv[0]);
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    int num_urls = argc - 1;
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_urls);
    if (!threads) {
        fprintf(stderr, "Memory cannot allocate to the task\n");
        curl_global_cleanup();
        return 1;
    }

    ThreadData **all_thread_data = (ThreadData **)malloc(sizeof(ThreadData *) * num_urls);
    if (!all_thread_data) {
        fprintf(stderr, "No memory for thread data pointers\n");
        free(threads);
        curl_global_cleanup();
        return 1;
    }

    //Create threads iteratively from each URL
    for (int i = 0; i < num_urls; i++) {
        //Allocating memory for thread data
        ThreadData *data = (ThreadData *)malloc(sizeof(ThreadData));
        if (!data) {
            fprintf(stderr, "Error\n");
            for (int j = 0; j < i; j++) {
            }
            free(threads);
            free(all_thread_data);
            curl_global_cleanup();
            return 1;
        }

        //Duplicating URL string for thread safety
        data->url = strdup(argv[i + 1]);
        if (!data->url) {
            fprintf(stderr, "Can't duplicate URL\n");
            free(data);
            free(threads);
            free(all_thread_data);
            curl_global_cleanup();
            return 1;
        }

        //Generating unique filename for every URL
        data->filename = (char *)malloc(strlen("output_") + strlen(data->url) + strlen(".html") + 10);
        if (!data->filename) {
            fprintf(stderr, "No memory allwowed for file at the moment\n");
            free(data->url);
            free(data);
            free(threads);
            free(all_thread_data);
            curl_global_cleanup();
            return 1;
        }
        sprintf(data->filename, "output_%d.html", i + 1);

        all_thread_data[i] = data;

        int rc = pthread_create(&threads[i], NULL, fetch_url, (void *)data);
        if (rc) {
            fprintf(stderr, "Error: Failed to create thread for URL %s. Return code: %d\n", data->url, rc);
            //freeing data allocated from thread
            free(data->url);
            free(data->filename);
            free(data);
            // Cleanup already created threads and data before exiting
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(all_thread_data);
            curl_global_cleanup();
            return 1;
        }
    }

    //Waiting for all threads to complete
    for (int i = 0; i < num_urls; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All web scraping threads have completed.\n");

    //Cleaning up allocated memory
    free(threads);
    free(all_thread_data);

    //environement scraping
    curl_global_cleanup();

    return 0;
}
