#!/bin/bash

POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
  case $1 in
    -t|--target)
      TARGET="$2"
      shift # past argument
      shift # past value
      ;;
    -e|--entry_point)
      ENTRYPOINT="$2"
      shift # past argument
      shift # past value
      ;;
    -s|--source)
      SOURCE="$2"
      shift # past argument
      shift # past value
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done

set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

CMD="sudo docker run --rm -v $(pwd):$(pwd) -w $(pwd) gwihlidal/dxc -T ${TARGET} -E ${ENTRYPOINT} ${SOURCE}"
echo CMD: ${CMD}
${CMD}
