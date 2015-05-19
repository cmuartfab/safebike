clear
close all

data1_path = '../data/cross 1/';
data2_path = '../data_bike/cross 1/';

% data1_time_filename = data1_path + 'piksi_gps_time.csv';
% data2_time_filename = data2_path + 'piksi_gps_time.csv';
% data1_time = csvread(data1_time_filename, 1, 0);
% data2_time = csvread(data2_time_filename, 1, 0);

data1_pos_llh_filename = strcat(data1_path, 'piksi_pos_llh.csv');
data2_pos_llh_filename = strcat(data2_path, 'piksi_pos_llh.csv');
data1_pos_llh = csvread(data1_pos_llh_filename, 1, 0);
data2_pos_llh = csvread(data2_pos_llh_filename, 1, 0);

data1_offset = 0;
data2_offset = 62;
% calculate the longest useable section of data based on the row sizes
data_length = min(size(data1_pos_llh,1)-data1_offset, size(data2_pos_llh,1)-data2_offset)
% trim data. remember indexes must start at 1
data1_pos_llh = data1_pos_llh(data1_offset+1:(data1_offset+data_length),:);
data2_pos_llh = data2_pos_llh(data2_offset+1:(data2_offset+data_length),:);

pos_llh_time = data1_pos_llh(:,1)
pos_llh_diff = data2_pos_llh - data1_pos_llh;
figure()
plot(pos_llh_diff(:,2), pos_llh_diff(:,3))
title('GPS Single Point Difference', 'FontSize', 24)
xlabel('Latitude (deg)', 'FontSize', 18)
ylabel('Longitude (deg)', 'FontSize', 18)

figure();
plot(pos_llh_time, pos_llh_diff(:,2))
title('GPS Single Point Difference')
xlabel('GPS ToW (deg)')
ylabel('Latitude (deg)')

figure();
plot(pos_llh_time, pos_llh_diff(:,3))
title('GPS Single Point Difference')
xlabel('GPS ToW (deg)')
ylabel('Longitude (deg)')
