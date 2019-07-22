#!/usr/bin/env bash

source ./commons

python_path="./python_functions"

# process pcap files
#
# use tshark to to process pcap files and filter out
# signal strength, signal strength on the antennas, actual datarate,
# mcs rate, relative time stamp and sequence number
#
# Arguments:
# input_dir - directory with pcap files
# output_dir - directory for processed data
#
process_pcaps()
{
	input_dir=$1
	output_dir=$2

	for pcap in $(ls ${input_dir}/*.pcap)
	do
		if [[ $(tcpdump -r $pcap &>/dev/null; echo $?) -eq 0 ]]; then
			echo '#time seq_number datarate mcs_index signal' > \
				${output_dir}/cleaned_$(basename $pcap).csv
			tshark -r $pcap -R \
				"wlan_radio.signal_dbm<0&&wlan.fc.type==2"\
				-T fields \
				-e frame.time_relative \
				-e radiotap.mactime \
				-e wlan.seq \
				-e radiotap.datarate -2 \
				-e radiotap.mcs.index \
				-e radiotap.dbm_antsignal \
				| sed 's/,/ /g' \
				| awk '{print $1, $3, $4, $5, $6 ,$7, $8}' \
				>> ${output_dir}/cleaned_$(basename $pcap).csv
		else
			printf "${WARNING} could not process $pcap\n"
		fi
	done
}

# process iperf logs
#
# get received/send packets and selected MCS rate
#
# Arguments:
# input_dir - directory with iperf files
# output_dir - directory for processed data
#
process_iperf()
{
	input_dir=$1
	output_dir=$2

	out_prefix="cleaned_"

	#iterate over client iperf log files and store send packages/datarates
	for client_log in $(ls ${input_dir}/con-test-client-run*.log)
	do
		printf "${INFO} processing $client_log\n"
		echo '#transfer bitrate send' > \
			${output_dir}/${out_prefix}${client_log}
		awk -F'[ /]' '{ print $7, $11, $19}' >> \
			${output_dir}/${out_prefix}${client_log}
	done
}


# plot data from cleaned data
#
# Arguments:
# input_dir - directory containing processed data
#
plot_data()
{
	input_dir=$1
	plot_path=${input_dir}/plots
	
	if [ ! -f $plot_path ]; then
		printf "${INFO} Creating plot directory ${plot_path}\n"
		mkdir -p $plot_path
	fi

	# plot signal strength based plots
	for pcap in $(ls ${input_dir}/*.pcap.csv)
	do
		python3 ${python_path}/plot_signal.py $pcap $plot_path
	done
}


# main function
#
# Arguments:
# args - command line arguments
#
main()
{
	args=$@
	options=c:hi:o:vV
	loptions=config:,help,output:,input:,verbose,version

	config_path="con-test.conf"
	log_path="con-test_cleaned_data"
	input_dir="con-test_logs"

	! parsed=$(getopt --options=$options --longoptions=$loptions \
		   --name "$0" -- $args)
	if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
		exit $FAILURE
	fi

	eval set -- "$parsed"

	while true; do
		case "$1" in
			-c | --config)
				config_path=$(echo $2 | sed 's:/*$::')
				export config_path
				shift 2
				;;
			-h | --help)
				call_help
				exit $SUCCESS
				;;
			-i | --input)
				input_dir=$(echo $2 | sed 's:/*$::')
				export input_dir
				shift 2
				;;
			-o | --output)
				log_path=$(echo $2 | sed 's:/*$::')
				export log_path
				shift 2
				;;
			-v | --verbose)
				VERBOSE=y
				shift
				;;
			-V | --version)
				print_version
				exit $SUCCESS
				;;
			--)
				shift
				break
				;;
			*)
				break
				;;
		esac
	done

	printf "${INFO} Starting preparation of data in $input_dir\n"
	printf "\t\twriting to $log_path\n"

	if [ ! -f "$config_path" ]; then
		printf "${ERROR} $config_path does not exist, or can not be accessed"
		exit $FAILURE
	fi

	if [ ! -d "$log_path" ]; then
		printf "${WARN} $log_path does not exist, creating it\n"
		mkdir -p $log_path
	fi

	source $config_path

	process_pcaps $input_dir $log_path
	process_iperf $input_dir $log_path
	plot_data $log_path

}

main $@
