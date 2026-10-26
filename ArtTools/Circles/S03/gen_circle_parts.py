"""Reproducible controller-free parts from copied r3 specs; no production writes."""
from pathlib import Path
from copy import deepcopy
import argparse
import json
import os
import subprocess
import sys
from gen_derived import ROOT, GEN, OUT, read_spec, serialize
from preservation import snapshot, digest

PARTS = {
    'Charge_Circle_OuterBand_Static': ['CC_Outer', 'CC_Band'],
    'Charge_Circle_HebrewBand_Static': ['CC_Band'],
    'Charge_Circle_OuterRing_Static': ['CC_Outer']
}
BASELINE = ROOT/'work/external-baseline-main-s02d-v4.json'


def protected_gate():
    before = json.loads(BASELINE.read_text(encoding='utf-8'))
    after = snapshot()
    a = {(r['scope'], r['path']): r for r in before['files']}
    b = {(r['scope'], r['path']): r for r in after['files']}
    all_changes = [{'path': k[1], 'before': a.get(k), 'after': b.get(k)}
               for k in sorted(a.keys() | b.keys()) if a.get(k) != b.get(k)]
    document_scope = ROOT/'work/evidence-markdown-scope-v6.json'
    assert document_scope.is_file(), 'Separate-record authority must be recorded before applying the documentation exception'
    evidence_root = ROOT.parents[1]/'evidence'
    def external_evidence_markdown(path):
        p = Path(path)
        return p.is_relative_to(evidence_root) and p.suffix.lower() == '.md'
    observed_documents = [c for c in all_changes if external_evidence_markdown(c['path'])]
    changes = [c for c in all_changes if not external_evidence_markdown(c['path'])]
    kept_changes = [r for r in before['preserved_derived_nifs'] if digest(Path(r['path'])) != r['sha256']]
    dirs_changed = before['mo2_directories'] != after['mo2_directories']
    result = {'status': 'FAIL' if changes or kept_changes or dirs_changed else 'STATIC QA PASS',
              'baseline': str(BASELINE),
              'protected_file_count': sum(not external_evidence_markdown(r['path']) for r in after['files']),
              'external_inventory_file_count': len(after['files']),
              'observed_evidence_markdown_changes': observed_documents,
              'documentation_scope_authority': str(document_scope),
              'byte_or_metadata_changes': changes, 'existing_nif_changes': kept_changes,
              'mo2_directory_structure_changed': dirs_changed,
              'approved_exception': before['approved_added_file'],
              'approved_main_authority': before['authority'],
              'scope': 'Strict workspace/MO2 SHA256,size,mtime except user-authorized external evidence/**/*.md observations; task root, junctions/symlinks and .git excluded. Original assets, all non-evidence Markdown, production/build/deploy and MO2 directories remain strict.'}
    (ROOT/'reports/PARTS_V3_IMMUTABILITY_GATE.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    (ROOT/'reports/EXTERNAL_EVIDENCE_MARKDOWN_OBSERVATIONS.json').write_text(json.dumps({
        'authority': str(document_scope), 'baseline': str(BASELINE),
        'note': 'Read-only observations of concurrent external documents. These are not Codex output files. No source document was modified by this task.',
        'changes': observed_documents}, indent=2), encoding='utf-8')
    (ROOT/'work/parts-v3-after.json').write_text(json.dumps(after, indent=2), encoding='utf-8')
    if result['status'] == 'FAIL':
        raise SystemExit('STOP: new protected drift beyond user-approved S02D Main baseline. Do not repair or rebase.')
    return result


def spec_for(stem, tintable):
    source = GEN/'components'/('Charge_Circle_Static'+('_Tintable' if tintable else '')+'.nifspec')
    records = read_spec(source)
    keep = PARTS[stem]
    selected = [deepcopy(r) for r in records if
                (r['tokens'][0] == 'NODE' and r['tokens'][1] in keep) or
                (r['tokens'][0] == 'MESH' and r['tokens'][2] in keep)]
    assert len(selected) == len(keep)*2
    return source, [{'tokens': ['ROOT', 'CC_Root'], 'data': []}, *selected]


def build(spec, destination):
    assert spec.resolve().is_relative_to(ROOT) and destination.resolve().is_relative_to(ROOT)
    destination.parent.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env['TMP'] = env['TEMP'] = str(ROOT/'work/tmp')
    r = subprocess.run([str(ROOT/'work/build/nifbuild.exe'), str(spec), str(destination)],
                       capture_output=True, text=True, env=env)
    if r.returncode:
        raise RuntimeError(r.stdout+r.stderr)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--repro', action='store_true', help='Rebuild under work/parts_repro only and compare to authoritative final files')
    args = parser.parse_args()
    assert (ROOT/'reports/CIRCLE_PARTS_ASSEMBLY_DECISION.md').is_file()
    protected_gate()
    specs = ROOT/'work/parts_specs'
    specs.mkdir(parents=True, exist_ok=True)
    assets = []
    for stem, layers in PARTS.items():
        for tintable in [False, True]:
            name = stem+('_Tintable' if tintable else '')+'.nif'
            source, records = spec_for(stem, tintable)
            spec = specs/(Path(name).stem+'.nifspec')
            spec.write_text(serialize(records), encoding='utf-8')
            dest = OUT/'Common'/name
            repro = ROOT/'work/parts_repro'/name
            if not args.repro:
                assert not dest.exists(), 'Final file already exists; use --repro for read-only comparison'
                build(spec, dest)
            else:
                assert dest.is_file(), 'No authoritative final file available for comparison'
            build(spec, repro)
            assert digest(dest) == digest(repro), 'Reproduction differs'
            assets.append({'filename': name, 'relative_output_path': dest.relative_to(ROOT).as_posix(),
                           'absolute_output_path': str(dest), 'purpose': 'Reusable independently spawned '+', '.join(layers),
                           'source_components': [source.relative_to(ROOT).as_posix()],
                           'source_spec_sha256': digest(source), 'included_nodes': layers,
                           'tintable': tintable, 'status': ['GENERATED', 'RUNTIME UNVERIFIED'],
                           'byte_size': dest.stat().st_size, 'sha256': digest(dest),
                           'root_node': 'CC_Root', 'nifspec': spec.relative_to(ROOT).as_posix(),
                           'spec_sha256': digest(spec), 'reproduction': 'Second isolated build SHA256 identical'})
            print('GENERATED', name, dest.stat().st_size, digest(dest))
    if not args.repro:
        (ROOT/'work/parts-generated-assets.json').write_text(json.dumps(assets, indent=2), encoding='utf-8')
    protected_gate()
    print('6 parts built twice under output authority; no original/source/kept-NIF changes.')


if __name__ == '__main__':
    main()
