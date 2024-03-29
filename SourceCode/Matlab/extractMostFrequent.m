% Extracts the n most frequent words from the annotation structure
% D.Modolo & T.Kooi 2011

function [word_array,freqs] = extractMostFrequent(n)

%load annotation
load allAttributes.mat;
%load dictionary and frequency count
load dictionary.mat;

word_ids = zeros(1,2);

global frequencies,word_ids;

frequencies = zeros(size(words));
prefixes = cell(size(words));

attributes = struct2cell(lineAtts);

for i=1:size(attributes,1)
    line = struct2cell(attributes{i});
    for j=1:size(line,1)
        if(line{j}.id(1) ~='a'), break; end
            processLine(line{j}.text,line{j}.id,words);
    end
end

%n_most_frequent is an array of word indices
n_most_frequent = getMostFrequent(n);
word_array(:,1) = words(n_most_frequent(:,1));
freqs = n_most_frequent(:,2);

images_to_use = zeros(1,1);

%This is extremely inefficient but we'll only have to run this once, please
%excuse me for this
for i=1:size(word_ids,1)
    for j=1:50
        if(word_ids(i,1) == n_most_frequent(:,1))
            images_to_use(size(images_to_use,1)) = word_ids(i,2)
        end
    end
end
    
    
end

function n_most_frequent = getMostFrequent(n)

    global frequencies;
    n_most_frequent = zeros(n,2);

    for i=1:n
        [y,index] = max(frequencies);
        n_most_frequent(i,1) = index;
        n_most_frequent(i,2) = frequencies(index);
        frequencies(index) = 0;
    end

end


function processLine(line,id,words)

global frequencies;
global word_ids;

remain = line;
    while true
       [word, remain] = strtok(remain);
       if isempty(word),  break;  end
           [y,index] = max(strcmp(words,word));
           frequencies(index) = frequencies(index)+1;
           last_element = size(word_ids,1)
           new_id = [index,id];
           word_ids(last_element+1,:) = new_id;
    end

end
